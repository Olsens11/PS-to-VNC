/*
 * File synopsis:
 * Implements Wire Proof 4C: the P4 rider-isolation transaction with real
 * bounded RFB/AUDIO logical channels and the real synchronous PCM playback
 * core, while exactly one dedicated EE thread owns all physical Wire send and
 * receive calls.
 *
 * Thread topology under test:
 *
 *   Transport physical-I/O EE thread : priority 63
 *   RFB domain EE thread             : priority 64
 *   PCM domain EE thread             : priority 65
 *   MPEG domain EE thread            : priority 67 when active, inactive here
 *
 * These values are the best-known tested scheduling baseline, not a claim of
 * globally optimal or permanent priorities.
 *
 * Real mechanisms reused:
 *   - Transport physical_stream framing / sequence ownership;
 *   - Transport bounded RFB channel;
 *   - Transport bounded AUDIO channel;
 *   - RFB's public bridge;
 *   - the synchronous PCM playback core;
 *   - the qualified PS2 network path.
 *
 * Development apparatus:
 *   - proof-local synchronization around the standalone logical channel owners;
 *   - a single bounded outbound-work rendezvous from domain threads to the
 *     physical-I/O owner;
 *   - deterministic P4 transaction scheduling;
 *   - injected PCM service-play failure;
 *   - proof-local public Transport adapters binding the clean domain bridges to
 *     the standalone channel owners.
 *
 * This file does not modify production Transport and does not claim the final
 * production outbound-queue/event-loop implementation. It tests whether the
 * P4 Q8/Q11 rider-isolation transaction remains live when separate domain EE
 * threads are retained but all physical socket I/O is confined to one EE
 * execution context.
 */

#include <debug.h>
#include <kernel.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "audio/playback.h"
#include "config/profile.h"
#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "rfb/bridge.h"
#include "transport/audio_channel.h"
#include "transport/bridge.h"
#include "transport/physical_stream.h"
#include "transport/protocol.h"
#include "transport/rfb_channel.h"
#include "transport/transport.h"

#include "wire_establish_protocol.h"

#define P4C_TRANSPORT_IO_PRIORITY 63
#define P4C_RFB_PRIORITY          64
#define P4C_PCM_PRIORITY          65
#define P4C_MPEG_PRIORITY         67

#define P4C_THREAD_STACK_BYTES    16384

#define P4C_RFB_CAPACITY          32u
#define P4C_AUDIO_CAPACITY        16u
#define P4C_MAX_DATA_PAYLOAD      16u
#define P4C_AUDIO_TRIGGER_BYTES   4u
#define P4C_OUTBOUND_RFB_BYTES    37u

typedef enum p4c_outbound_kind {
    P4C_OUTBOUND_NONE = 0,
    P4C_OUTBOUND_CREDIT = 1,
    P4C_OUTBOUND_RFB_DATA = 2
} p4c_outbound_kind_t;

typedef struct p4c_outbound_work {
    p4c_outbound_kind_t kind;
    uint8_t channel;
    uint32_t credit_amount;
    uint8_t data[P4C_OUTBOUND_RFB_BYTES];
    size_t data_length;
    int result;
} p4c_outbound_work_t;

typedef struct p4c_audio_service_state {
    int initialize_calls;
    int format_calls;
    int volume_calls;
    int wait_calls;
    int play_calls;
    int stop_calls;
    int trigger_bytes_match;
} p4c_audio_service_state_t;

static pstvnc_transport_physical_stream_t p4c_stream;
static pstvnc_transport_rfb_channel_t p4c_rfb_channel;
static pstvnc_transport_audio_channel_t p4c_audio_channel;

static uint8_t p4c_rfb_storage[P4C_RFB_CAPACITY];
static uint8_t p4c_audio_storage[P4C_AUDIO_CAPACITY];

static unsigned char p4c_io_stack[P4C_THREAD_STACK_BYTES]
    __attribute__((aligned(16)));
static unsigned char p4c_rfb_stack[P4C_THREAD_STACK_BYTES]
    __attribute__((aligned(16)));
static unsigned char p4c_pcm_stack[P4C_THREAD_STACK_BYTES]
    __attribute__((aligned(16)));

static int p4c_io_thread_id = -1;
static int p4c_rfb_thread_id = -1;
static int p4c_pcm_thread_id = -1;

static int p4c_rfb_mutex = -1;
static int p4c_audio_mutex = -1;
static int p4c_audio_activity = -1;

static int p4c_pcm_start = -1;
static int p4c_pcm_failure_ready = -1;

static int p4c_rfb_first_gate = -1;
static int p4c_rfb_second_gate = -1;

static int p4c_outbound_slot = -1;
static int p4c_outbound_ready = -1;
static int p4c_outbound_done = -1;

static int p4c_io_done = -1;
static int p4c_rfb_done = -1;
static int p4c_pcm_done = -1;

static volatile int p4c_failed;
static volatile int p4c_wire_active;
static volatile int p4c_io_pass;
static volatile int p4c_rfb_pass;
static volatile int p4c_pcm_expected_failure;
static volatile int p4c_rfb_full_during_audio_progress;
static volatile int p4c_audio_full_after_failure;

static volatile uint32_t p4c_audio_generation;

static uint32_t p4c_session_id;

static p4c_outbound_work_t p4c_outbound_work;
static p4c_audio_service_state_t p4c_audio_service;

static const uint8_t p4c_expected_rfb_first[P4C_RFB_CAPACITY] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

static const uint8_t p4c_expected_rfb_second[P4C_RFB_CAPACITY] = {
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f
};

static const uint8_t p4c_expected_audio_trigger[P4C_AUDIO_TRIGGER_BYTES] = {
    0xa1, 0xa2, 0xa3, 0xa4
};

static const uint8_t p4c_expected_audio_full[P4C_AUDIO_CAPACITY] = {
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
    0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf
};

static const uint8_t p4c_outbound_rfb[P4C_OUTBOUND_RFB_BYTES] = {
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4
};

static void p4c_print(const char *message)
{
    printf("%s\n", message);
    scr_printf("%s\n", message);
}

static int p4c_create_semaphore(int initial_count, int maximum_count)
{
    ee_sema_t semaphore;

    memset(&semaphore, 0, sizeof(semaphore));
    semaphore.init_count = initial_count;
    semaphore.max_count = maximum_count;
    semaphore.option = 0;

    return CreateSema(&semaphore);
}

static int p4c_create_thread(
    void (*entry)(void *),
    void *stack,
    size_t stack_size,
    int priority)
{
    ee_thread_t thread;

    memset(&thread, 0, sizeof(thread));

    thread.func = (void *)entry;
    thread.stack = stack;
    thread.stack_size = (int)stack_size;
    thread.gp_reg = &_gp;
    thread.initial_priority = priority;
    thread.attr = 0;
    thread.option = 0;

    return CreateThread(&thread);
}

static void p4c_signal_if_valid(int semaphore_id)
{
    if (semaphore_id >= 0)
        (void)SignalSema(semaphore_id);
}

static void p4c_mark_failed(const char *stage)
{
    char line[128];

    p4c_failed = 1;

    if (stage != NULL) {
        snprintf(line, sizeof(line), "FAIL %s", stage);
        p4c_print(line);
    }

    /*
     * Wake every proof-local wait point. Signal failures are intentionally
     * ignored here because some semaphores may already contain their one token.
     */
    p4c_signal_if_valid(p4c_audio_activity);
    p4c_signal_if_valid(p4c_pcm_start);
    p4c_signal_if_valid(p4c_pcm_failure_ready);
    p4c_signal_if_valid(p4c_rfb_first_gate);
    p4c_signal_if_valid(p4c_rfb_second_gate);
    p4c_signal_if_valid(p4c_outbound_ready);
    p4c_signal_if_valid(p4c_outbound_done);
}

static void p4c_write_be32(uint8_t destination[4], uint32_t value)
{
    destination[0] = (uint8_t)((value >> 24) & 0xffu);
    destination[1] = (uint8_t)((value >> 16) & 0xffu);
    destination[2] = (uint8_t)((value >> 8) & 0xffu);
    destination[3] = (uint8_t)(value & 0xffu);
}

static pstvnc_transport_result_t p4c_terminal_result(void)
{
    if (p4c_failed)
        return PSTVNC_TRANSPORT_FAILED;

    if (!p4c_wire_active)
        return PSTVNC_TRANSPORT_CLOSED;

    return PSTVNC_TRANSPORT_FAILED;
}

/*
 * Bounded domain -> physical-I/O rendezvous.
 *
 * Exactly one work item can be outstanding. That is sufficient for this
 * deterministic proof transaction and makes the capacity explicit rather than
 * allowing any domain thread to touch the socket.
 */
static int p4c_submit_credit(uint8_t channel, uint32_t amount)
{
    int result;

    if (p4c_failed)
        return 0;

    if (WaitSema(p4c_outbound_slot) < 0)
        return 0;

    if (p4c_failed) {
        (void)SignalSema(p4c_outbound_slot);
        return 0;
    }

    memset(&p4c_outbound_work, 0, sizeof(p4c_outbound_work));
    p4c_outbound_work.kind = P4C_OUTBOUND_CREDIT;
    p4c_outbound_work.channel = channel;
    p4c_outbound_work.credit_amount = amount;

    if (SignalSema(p4c_outbound_ready) < 0) {
        (void)SignalSema(p4c_outbound_slot);
        return 0;
    }

    if (WaitSema(p4c_outbound_done) < 0) {
        (void)SignalSema(p4c_outbound_slot);
        return 0;
    }

    result = p4c_outbound_work.result;

    if (SignalSema(p4c_outbound_slot) < 0)
        return 0;

    return result;
}

static int p4c_submit_rfb_data(const void *buffer, size_t count)
{
    int result;

    if (buffer == NULL || count == 0u ||
        count > sizeof(p4c_outbound_work.data) ||
        p4c_failed)
        return 0;

    if (WaitSema(p4c_outbound_slot) < 0)
        return 0;

    if (p4c_failed) {
        (void)SignalSema(p4c_outbound_slot);
        return 0;
    }

    memset(&p4c_outbound_work, 0, sizeof(p4c_outbound_work));
    p4c_outbound_work.kind = P4C_OUTBOUND_RFB_DATA;
    p4c_outbound_work.channel = PSTVNC_TRANSPORT_CHANNEL_RFB;
    p4c_outbound_work.data_length = count;
    memcpy(p4c_outbound_work.data, buffer, count);

    if (SignalSema(p4c_outbound_ready) < 0) {
        (void)SignalSema(p4c_outbound_slot);
        return 0;
    }

    if (WaitSema(p4c_outbound_done) < 0) {
        (void)SignalSema(p4c_outbound_slot);
        return 0;
    }

    result = p4c_outbound_work.result;

    if (SignalSema(p4c_outbound_slot) < 0)
        return 0;

    return result;
}

/*
 * Proof-local public Transport adapters.
 *
 * These bind RFB/PCM domain-facing clean interfaces to the real standalone
 * logical channel owners. They do not perform physical socket I/O.
 */

pstvnc_transport_result_t pstvnc_transport_rfb_read_exact(
    void *buffer,
    size_t count)
{
    int read_result;

    if (buffer == NULL || count == 0u)
        return PSTVNC_TRANSPORT_INVALID;

    if (!p4c_wire_active)
        return p4c_terminal_result();

    if (WaitSema(p4c_rfb_mutex) < 0)
        return PSTVNC_TRANSPORT_FAILED;

    read_result = pstvnc_transport_rfb_channel_read_exact(
        &p4c_rfb_channel,
        buffer,
        count);

    if (SignalSema(p4c_rfb_mutex) < 0)
        return PSTVNC_TRANSPORT_FAILED;

    if (read_result != 0)
        return p4c_terminal_result();

    if (!p4c_submit_credit(
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            (uint32_t)count))
        return PSTVNC_TRANSPORT_FAILED;

    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_rfb_poll_receive(void)
{
    size_t available;

    if (!p4c_wire_active)
        return p4c_terminal_result();

    if (WaitSema(p4c_rfb_mutex) < 0)
        return PSTVNC_TRANSPORT_FAILED;

    available = pstvnc_transport_rfb_channel_available(&p4c_rfb_channel);

    if (SignalSema(p4c_rfb_mutex) < 0)
        return PSTVNC_TRANSPORT_FAILED;

    return available != 0u
        ? PSTVNC_TRANSPORT_OK
        : PSTVNC_TRANSPORT_WOULD_BLOCK;
}

pstvnc_transport_result_t pstvnc_transport_rfb_write_exact(
    const void *buffer,
    size_t count)
{
    if (!p4c_wire_active)
        return p4c_terminal_result();

    if (!p4c_submit_rfb_data(buffer, count))
        return PSTVNC_TRANSPORT_FAILED;

    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_rfb_quiesce_requested(void)
{
    return PSTVNC_TRANSPORT_INVALID;
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_boundary(void)
{
    return PSTVNC_TRANSPORT_INVALID;
}

pstvnc_transport_result_t pstvnc_transport_rfb_wait_quiesce_commit(void)
{
    return PSTVNC_TRANSPORT_INVALID;
}

pstvnc_transport_result_t pstvnc_transport_rfb_snapshot_quiesce_residual(
    size_t *residual_count)
{
    if (residual_count != NULL)
        *residual_count = 0u;

    return PSTVNC_TRANSPORT_INVALID;
}

pstvnc_transport_result_t pstvnc_transport_rfb_discard_quiesce_residual(
    size_t expected_count,
    size_t *discarded_count)
{
    (void)expected_count;

    if (discarded_count != NULL)
        *discarded_count = 0u;

    return PSTVNC_TRANSPORT_INVALID;
}

pstvnc_transport_result_t pstvnc_transport_rfb_send_quiesce_complete(void)
{
    return PSTVNC_TRANSPORT_INVALID;
}

pstvnc_transport_result_t pstvnc_transport_audio_read_available(
    void *buffer,
    size_t maximum_count,
    size_t *read_count)
{
    size_t count;
    int producer_done;

    if (read_count == NULL ||
        (buffer == NULL && maximum_count != 0u))
        return PSTVNC_TRANSPORT_INVALID;

    *read_count = 0u;

    if (!p4c_wire_active)
        return p4c_terminal_result();

    if (WaitSema(p4c_audio_mutex) < 0)
        return PSTVNC_TRANSPORT_FAILED;

    producer_done =
        pstvnc_transport_audio_channel_producer_done(&p4c_audio_channel);

    count = pstvnc_transport_audio_channel_read_available(
        &p4c_audio_channel,
        (uint8_t *)buffer,
        maximum_count);

    if (SignalSema(p4c_audio_mutex) < 0)
        return PSTVNC_TRANSPORT_FAILED;

    if (count == 0u)
        return producer_done
            ? PSTVNC_TRANSPORT_EXHAUSTED
            : PSTVNC_TRANSPORT_WOULD_BLOCK;

    if (!p4c_submit_credit(
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            (uint32_t)count))
        return PSTVNC_TRANSPORT_FAILED;

    *read_count = count;
    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_audio_status(
    size_t *available_count,
    int *producer_done)
{
    if (available_count == NULL || producer_done == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    if (!p4c_wire_active)
        return p4c_terminal_result();

    if (WaitSema(p4c_audio_mutex) < 0)
        return PSTVNC_TRANSPORT_FAILED;

    *available_count =
        pstvnc_transport_audio_channel_available(&p4c_audio_channel);
    *producer_done =
        pstvnc_transport_audio_channel_producer_done(&p4c_audio_channel);

    if (SignalSema(p4c_audio_mutex) < 0)
        return PSTVNC_TRANSPORT_FAILED;

    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_audio_activity_snapshot(
    uint32_t *activity_sequence)
{
    if (activity_sequence == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    if (!p4c_wire_active)
        return p4c_terminal_result();

    if (WaitSema(p4c_audio_mutex) < 0)
        return PSTVNC_TRANSPORT_FAILED;

    *activity_sequence = p4c_audio_generation;

    if (SignalSema(p4c_audio_mutex) < 0)
        return PSTVNC_TRANSPORT_FAILED;

    return PSTVNC_TRANSPORT_OK;
}

pstvnc_transport_result_t pstvnc_transport_audio_wait_activity(
    uint32_t *activity_sequence)
{
    uint32_t observed;

    if (activity_sequence == NULL)
        return PSTVNC_TRANSPORT_INVALID;

    for (;;) {
        if (p4c_failed)
            return PSTVNC_TRANSPORT_FAILED;

        if (!p4c_wire_active)
            return PSTVNC_TRANSPORT_CLOSED;

        if (WaitSema(p4c_audio_mutex) < 0)
            return PSTVNC_TRANSPORT_FAILED;

        observed = p4c_audio_generation;

        if (SignalSema(p4c_audio_mutex) < 0)
            return PSTVNC_TRANSPORT_FAILED;

        if (observed != *activity_sequence) {
            *activity_sequence = observed;
            return PSTVNC_TRANSPORT_OK;
        }

        if (WaitSema(p4c_audio_activity) < 0)
            return PSTVNC_TRANSPORT_FAILED;
    }
}

/*
 * Deterministic injected PCM service.
 *
 * The first real AUDIO payload reaches the synchronous playback core. Service
 * play then fails deliberately. stop_audio() must still run and succeed.
 */

static int p4c_audio_initialize(void *context)
{
    p4c_audio_service_state_t *state =
        (p4c_audio_service_state_t *)context;

    state->initialize_calls++;
    return 0;
}

static int p4c_audio_set_format(
    void *context,
    uint32_t rate_hz,
    uint32_t channels,
    uint32_t bits_per_sample)
{
    p4c_audio_service_state_t *state =
        (p4c_audio_service_state_t *)context;

    state->format_calls++;

    return rate_hz == 44100u &&
        channels == 2u &&
        bits_per_sample == 16u
        ? 0
        : -1;
}

static int p4c_audio_set_volume(
    void *context,
    uint32_t volume_percent)
{
    p4c_audio_service_state_t *state =
        (p4c_audio_service_state_t *)context;

    state->volume_calls++;
    return volume_percent == 73u ? 0 : -1;
}

static int p4c_audio_wait_audio(
    void *context,
    size_t byte_count)
{
    p4c_audio_service_state_t *state =
        (p4c_audio_service_state_t *)context;

    state->wait_calls++;
    return byte_count == P4C_AUDIO_TRIGGER_BYTES ? 0 : -1;
}

static int p4c_audio_play_audio(
    void *context,
    const uint8_t *bytes,
    size_t byte_count)
{
    p4c_audio_service_state_t *state =
        (p4c_audio_service_state_t *)context;

    state->play_calls++;

    if (bytes != NULL &&
        byte_count == sizeof(p4c_expected_audio_trigger) &&
        memcmp(
            bytes,
            p4c_expected_audio_trigger,
            sizeof(p4c_expected_audio_trigger)) == 0)
        state->trigger_bytes_match = 1;

    return -1;
}

static int p4c_audio_stop_audio(void *context)
{
    p4c_audio_service_state_t *state =
        (p4c_audio_service_state_t *)context;

    state->stop_calls++;
    return 0;
}

/*
 * IO-OWNER PHYSICAL CALLS BEGIN
 *
 * No RFB/PCM domain routine below this comment is permitted. Every
 * physical_stream send/receive call in this source must remain inside this
 * physical-I/O-owner section.
 */

static int p4c_io_send_credit(uint8_t channel, uint32_t amount)
{
    uint8_t payload[PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE];

    p4c_write_be32(payload, amount);

    return pstvnc_transport_physical_stream_send_frame(
        &p4c_stream,
        PSTVNC_TRANSPORT_FRAME_CREDIT,
        channel,
        0u,
        payload,
        sizeof(payload));
}

static int p4c_io_send_heartbeat(void)
{
    return pstvnc_transport_physical_stream_send_frame(
        &p4c_stream,
        PSTVNC_TRANSPORT_FRAME_HEARTBEAT,
        PSTVNC_TRANSPORT_CHANNEL_CONTROL,
        0u,
        NULL,
        0u);
}

static int p4c_io_send_rfb_data(
    const uint8_t *bytes,
    size_t byte_count)
{
    size_t offset = 0u;

    if (bytes == NULL || byte_count == 0u)
        return 0;

    while (offset < byte_count) {
        size_t remaining = byte_count - offset;
        size_t count = remaining;

        if (count > P4C_MAX_DATA_PAYLOAD)
            count = P4C_MAX_DATA_PAYLOAD;

        if (!pstvnc_transport_physical_stream_send_frame(
                &p4c_stream,
                PSTVNC_TRANSPORT_FRAME_DATA,
                PSTVNC_TRANSPORT_CHANNEL_RFB,
                0u,
                bytes + offset,
                count))
            return 0;

        offset += count;
    }

    return 1;
}

static int p4c_io_establish_wire(uint32_t *session_id)
{
    uint8_t hello_payload[PSTVNC_WIRE_PROOF_HELLO_BYTES];
    uint8_t result_payload[PSTVNC_WIRE_PROOF_ACCEPT_BYTES];
    pstvnc_wire_proof_hello_t hello;
    pstvnc_wire_proof_accept_t acceptance;
    pstvnc_wire_proof_not_accepted_t rejection;
    pstvnc_transport_header_t header;

    if (session_id == NULL)
        return 0;

    *session_id = 0u;

    hello.wire_version = PSTVNC_WIRE_PROOF_WIRE_VERSION;
    hello.product_version = PSTVNC_WIRE_PROOF_PRODUCT_VERSION;

    if (!pstvnc_wire_proof_hello_encode(hello_payload, &hello))
        return 0;

    if (!pstvnc_transport_physical_stream_send_frame(
            &p4c_stream,
            PSTVNC_TRANSPORT_FRAME_HELLO,
            PSTVNC_TRANSPORT_CHANNEL_CONTROL,
            0u,
            hello_payload,
            sizeof(hello_payload)))
        return 0;

    if (!pstvnc_transport_physical_stream_receive_frame(
            &p4c_stream,
            &header,
            result_payload,
            sizeof(result_payload)))
        return 0;

    if (header.channel != PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
        header.flags != 0u ||
        header.payload_length != sizeof(result_payload))
        return 0;

    if (header.kind == PSTVNC_WIRE_PROOF_FRAME_ACCEPT) {
        if (!pstvnc_wire_proof_accept_decode(
                &acceptance,
                result_payload,
                sizeof(result_payload)) ||
            acceptance.session_id == 0u)
            return 0;

        *session_id = acceptance.session_id;
        return 1;
    }

    if (header.kind == PSTVNC_WIRE_PROOF_FRAME_NOT_ACCEPTED) {
        if (!pstvnc_wire_proof_not_accepted_decode(
                &rejection,
                result_payload,
                sizeof(result_payload)))
            return 0;

        return -(int)rejection.reason;
    }

    return 0;
}

static int p4c_io_receive_rfb_fragment(size_t expected_count)
{
    uint8_t payload[P4C_MAX_DATA_PAYLOAD];
    pstvnc_transport_header_t header;
    int result;

    if (expected_count == 0u ||
        expected_count > sizeof(payload))
        return 0;

    if (!pstvnc_transport_physical_stream_receive_frame(
            &p4c_stream,
            &header,
            payload,
            sizeof(payload)))
        return 0;

    if (header.kind != PSTVNC_TRANSPORT_FRAME_DATA ||
        header.channel != PSTVNC_TRANSPORT_CHANNEL_RFB ||
        header.flags != 0u ||
        header.payload_length != expected_count)
        return 0;

    if (WaitSema(p4c_rfb_mutex) < 0)
        return 0;

    result = pstvnc_transport_rfb_channel_commit(
        &p4c_rfb_channel,
        payload,
        expected_count);

    if (SignalSema(p4c_rfb_mutex) < 0)
        return 0;

    return result == 0;
}

static int p4c_io_receive_audio_fragment(
    const uint8_t *expected,
    size_t expected_count)
{
    uint8_t payload[P4C_MAX_DATA_PAYLOAD];
    pstvnc_transport_header_t header;
    int result;

    if (expected == NULL ||
        expected_count == 0u ||
        expected_count > sizeof(payload))
        return 0;

    if (!pstvnc_transport_physical_stream_receive_frame(
            &p4c_stream,
            &header,
            payload,
            sizeof(payload)))
        return 0;

    if (header.kind != PSTVNC_TRANSPORT_FRAME_DATA ||
        header.channel != PSTVNC_TRANSPORT_CHANNEL_AUDIO ||
        header.flags != 0u ||
        header.payload_length != expected_count ||
        memcmp(payload, expected, expected_count) != 0)
        return 0;

    if (WaitSema(p4c_audio_mutex) < 0)
        return 0;

    result = pstvnc_transport_audio_channel_commit_data(
        &p4c_audio_channel,
        payload,
        expected_count);

    if (result == 0)
        p4c_audio_generation++;

    if (SignalSema(p4c_audio_mutex) < 0)
        return 0;

    if (result != 0)
        return 0;

    if (SignalSema(p4c_audio_activity) < 0)
        return 0;

    return 1;
}

static int p4c_io_process_expected_work(
    p4c_outbound_kind_t expected_kind,
    uint8_t expected_channel,
    uint32_t expected_credit,
    size_t expected_data_length)
{
    int send_ok = 0;

    if (WaitSema(p4c_outbound_ready) < 0)
        return 0;

    if (p4c_failed) {
        p4c_outbound_work.result = 0;
        (void)SignalSema(p4c_outbound_done);
        return 0;
    }

    if (p4c_outbound_work.kind != expected_kind ||
        p4c_outbound_work.channel != expected_channel) {
        p4c_outbound_work.result = 0;
        (void)SignalSema(p4c_outbound_done);
        return 0;
    }

    if (expected_kind == P4C_OUTBOUND_CREDIT) {
        if (p4c_outbound_work.credit_amount != expected_credit) {
            p4c_outbound_work.result = 0;
            (void)SignalSema(p4c_outbound_done);
            return 0;
        }

        send_ok = p4c_io_send_credit(
            expected_channel,
            expected_credit);
    } else if (expected_kind == P4C_OUTBOUND_RFB_DATA) {
        if (p4c_outbound_work.data_length != expected_data_length) {
            p4c_outbound_work.result = 0;
            (void)SignalSema(p4c_outbound_done);
            return 0;
        }

        send_ok = p4c_io_send_rfb_data(
            p4c_outbound_work.data,
            p4c_outbound_work.data_length);
    }

    p4c_outbound_work.result = send_ok ? 1 : 0;

    if (SignalSema(p4c_outbound_done) < 0)
        return 0;

    return send_ok;
}

/*
 * IO-OWNER PHYSICAL CALLS END
 */

static void p4c_io_thread(void *argument)
{
    size_t available;
    char line[128];
    int establish_result;

    (void)argument;

    p4c_print("TRANSPORT_IO_PRIORITY=63");
    p4c_print("PHYSICAL_IO_OWNER=TRANSPORT_THREAD_ONLY");

    establish_result = p4c_io_establish_wire(&p4c_session_id);
    if (establish_result <= 0) {
        p4c_mark_failed("io_establishment");
        goto done;
    }

    p4c_wire_active = 1;

    snprintf(
        line,
        sizeof(line),
        "WIRE ACTIVE id=%u",
        (unsigned int)p4c_session_id);
    p4c_print(line);

    /*
     * PS2 sequences 2 and 3.
     */
    if (!p4c_io_send_credit(
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            P4C_RFB_CAPACITY) ||
        !p4c_io_send_credit(
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            P4C_AUDIO_CAPACITY)) {
        p4c_mark_failed("initial_credits");
        goto done;
    }

    p4c_print("INITIAL_CREDITS=32+16");

    if (SignalSema(p4c_pcm_start) < 0) {
        p4c_mark_failed("pcm_start_signal");
        goto done;
    }

    /*
     * Pi sequences 2 and 3. RFB worker remains gated, so the real bounded
     * channel reaches its complete 32-byte capacity.
     */
    if (!p4c_io_receive_rfb_fragment(16u) ||
        !p4c_io_receive_rfb_fragment(16u)) {
        p4c_mark_failed("first_rfb_receive");
        goto done;
    }

    if (WaitSema(p4c_rfb_mutex) < 0) {
        p4c_mark_failed("rfb_full_lock");
        goto done;
    }

    available =
        pstvnc_transport_rfb_channel_available(&p4c_rfb_channel);

    if (SignalSema(p4c_rfb_mutex) < 0 ||
        available != P4C_RFB_CAPACITY) {
        p4c_mark_failed("rfb_not_full");
        goto done;
    }

    p4c_rfb_full_during_audio_progress = 1;
    p4c_print("RFB_RELAY_FULL=32");

    /*
     * Pi sequence 4. PCM thread consumes these bytes through the real
     * synchronous playback core and requests four AUDIO credits through the
     * bounded outbound rendezvous.
     */
    if (!p4c_io_receive_audio_fragment(
            p4c_expected_audio_trigger,
            sizeof(p4c_expected_audio_trigger))) {
        p4c_mark_failed("audio_trigger_receive");
        goto done;
    }

    if (!p4c_io_process_expected_work(
            P4C_OUTBOUND_CREDIT,
            PSTVNC_TRANSPORT_CHANNEL_AUDIO,
            P4C_AUDIO_TRIGGER_BYTES,
            0u)) {
        p4c_mark_failed("audio_credit");
        goto done;
    }

    p4c_print("AUDIO_PROGRESS_WHILE_RFB_FULL=YES");

    /*
     * Pi sequence 5. PCM has already consumed the trigger. This refill occupies
     * the whole 16-byte AUDIO Relay after the expected playback failure.
     */
    if (!p4c_io_receive_audio_fragment(
            p4c_expected_audio_full,
            sizeof(p4c_expected_audio_full))) {
        p4c_mark_failed("audio_refill_receive");
        goto done;
    }

    if (WaitSema(p4c_pcm_failure_ready) < 0 ||
        p4c_failed ||
        !p4c_pcm_expected_failure) {
        p4c_mark_failed("pcm_failure_not_ready");
        goto done;
    }

    if (WaitSema(p4c_audio_mutex) < 0) {
        p4c_mark_failed("audio_full_lock");
        goto done;
    }

    available =
        pstvnc_transport_audio_channel_available(&p4c_audio_channel);

    if (SignalSema(p4c_audio_mutex) < 0 ||
        available != P4C_AUDIO_CAPACITY) {
        p4c_mark_failed("audio_not_full_after_failure");
        goto done;
    }

    p4c_audio_full_after_failure = 1;

    p4c_print("AUDIO_FAILURE=SERVICE_PLAY");
    p4c_print("AUDIO_CLEANUP=PASS");
    p4c_print("AUDIO_FAILURE_LOCAL=YES");
    p4c_print("AUDIO_RELAY_FULL_AFTER_FAILURE=16");

    /*
     * PS2 sequence 5: proof-only ordering marker after local PCM failure and
     * after the failed AUDIO rider is confirmed full.
     */
    if (!p4c_io_send_heartbeat()) {
        p4c_mark_failed("failure_marker");
        goto done;
    }

    p4c_print("AUDIO_FAILURE_MARKER_SENT=YES");

    /*
     * Allow the separate priority-64 RFB worker to consume the first real Relay
     * window. Its public bridge read queues the credit back to this I/O owner.
     */
    if (SignalSema(p4c_rfb_first_gate) < 0) {
        p4c_mark_failed("rfb_first_gate");
        goto done;
    }

    if (!p4c_io_process_expected_work(
            P4C_OUTBOUND_CREDIT,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            P4C_RFB_CAPACITY,
            0u)) {
        p4c_mark_failed("first_rfb_credit");
        goto done;
    }

    p4c_print("RFB_FIRST_DRAIN_AFTER_AUDIO_FAIL=32");

    /*
     * Pi sequences 6 and 7. The failed AUDIO Relay remains full while the
     * sibling RFB path receives a fresh 32-byte window.
     */
    if (!p4c_io_receive_rfb_fragment(16u) ||
        !p4c_io_receive_rfb_fragment(16u)) {
        p4c_mark_failed("second_rfb_receive");
        goto done;
    }

    p4c_print("RFB_PROGRESS_WITH_FAILED_AUDIO=YES");

    if (SignalSema(p4c_rfb_second_gate) < 0) {
        p4c_mark_failed("rfb_second_gate");
        goto done;
    }

    /*
     * PS2 sequence 7: exact boundary that failed in split-thread P4.
     */
    if (!p4c_io_process_expected_work(
            P4C_OUTBOUND_CREDIT,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            P4C_RFB_CAPACITY,
            0u)) {
        p4c_mark_failed("second_rfb_credit");
        goto done;
    }

    p4c_print("SEQ7_CREDIT_COMPLETE=YES");

    /*
     * PS2 sequences 8, 9, 10: RFB worker's exact-write request, fragmented only
     * by the physical-I/O owner.
     */
    if (!p4c_io_process_expected_work(
            P4C_OUTBOUND_RFB_DATA,
            PSTVNC_TRANSPORT_CHANNEL_RFB,
            0u,
            P4C_OUTBOUND_RFB_BYTES)) {
        p4c_mark_failed("outbound_rfb");
        goto done;
    }

    p4c_print("RFB_OUTBOUND_AFTER_AUDIO_FAIL=37");
    p4c_print("TRANSPORT_FAILED=NO");
    p4c_print("WIRE_HEALTHY_WITH_AUDIO_FAILED=YES");

    p4c_io_pass = 1;

done:
    p4c_wire_active = 0;
    p4c_signal_if_valid(p4c_audio_activity);

    /*
     * Successful hardware path releases the physical stream from the same I/O
     * owner that performed every physical send and receive.
     */
    (void)pstvnc_transport_physical_stream_release(&p4c_stream);

    if (p4c_io_pass)
        p4c_print("WIRE INACTIVE");

    p4c_signal_if_valid(p4c_io_done);
    ExitThread();
}

static void p4c_rfb_thread(void *argument)
{
    uint8_t first[P4C_RFB_CAPACITY];
    uint8_t second[P4C_RFB_CAPACITY];

    (void)argument;

    p4c_print("RFB_PRIORITY=64");

    if (WaitSema(p4c_rfb_first_gate) < 0 ||
        p4c_failed)
        goto failed;

    if (pstvnc_rfb_bridge_read_exact(
            first,
            sizeof(first)) != 0 ||
        memcmp(
            first,
            p4c_expected_rfb_first,
            sizeof(first)) != 0)
        goto failed;

    if (WaitSema(p4c_rfb_second_gate) < 0 ||
        p4c_failed)
        goto failed;

    if (pstvnc_rfb_bridge_read_exact(
            second,
            sizeof(second)) != 0 ||
        memcmp(
            second,
            p4c_expected_rfb_second,
            sizeof(second)) != 0)
        goto failed;

    if (pstvnc_rfb_bridge_write_exact(
            p4c_outbound_rfb,
            sizeof(p4c_outbound_rfb)) != 0)
        goto failed;

    p4c_rfb_pass = 1;
    p4c_print("RFB_DOMAIN_THREAD=PASS");
    p4c_signal_if_valid(p4c_rfb_done);
    ExitThread();
    return;

failed:
    if (!p4c_failed)
        p4c_mark_failed("rfb_domain");

    p4c_signal_if_valid(p4c_rfb_done);
    ExitThread();
}

static void p4c_pcm_thread(void *argument)
{
    pstvnc_config_pcm_profile_t profile;
    pstvnc_audio_service_ops_t service;
    pstvnc_audio_playback_report_t report;
    pstvnc_audio_playback_result_t result;
    uint8_t playback_buffer[P4C_AUDIO_TRIGGER_BYTES];

    (void)argument;

    memset(&profile, 0, sizeof(profile));
    memset(&service, 0, sizeof(service));
    memset(&report, 0, sizeof(report));
    memset(&p4c_audio_service, 0, sizeof(p4c_audio_service));

    p4c_print("PCM_PRIORITY=65");

    if (WaitSema(p4c_pcm_start) < 0 ||
        p4c_failed)
        goto failed;

    profile.rate_hz = 44100u;
    profile.channels = 2u;
    profile.bits_per_sample = 16u;
    profile.volume_percent = 73u;

    service.initialize = p4c_audio_initialize;
    service.set_format = p4c_audio_set_format;
    service.set_volume = p4c_audio_set_volume;
    service.wait_audio = p4c_audio_wait_audio;
    service.play_audio = p4c_audio_play_audio;
    service.stop_audio = p4c_audio_stop_audio;
    service.context = &p4c_audio_service;

    result = pstvnc_audio_playback_run(
        &profile,
        playback_buffer,
        sizeof(playback_buffer),
        &service,
        &report);

    if (result != PSTVNC_AUDIO_PLAYBACK_SERVICE_PLAY_FAILED ||
        !report.cleanup_attempted ||
        report.cleanup_failed ||
        p4c_audio_service.initialize_calls != 1 ||
        p4c_audio_service.format_calls != 1 ||
        p4c_audio_service.volume_calls != 1 ||
        p4c_audio_service.wait_calls != 1 ||
        p4c_audio_service.play_calls != 1 ||
        p4c_audio_service.stop_calls != 1 ||
        !p4c_audio_service.trigger_bytes_match)
        goto failed;

    p4c_pcm_expected_failure = 1;

    /*
     * Separate ordering token for the I/O owner plus independent thread-done
     * token for main's completion fence.
     */
    p4c_signal_if_valid(p4c_pcm_failure_ready);
    p4c_signal_if_valid(p4c_pcm_done);

    ExitThread();
    return;

failed:
    if (!p4c_failed)
        p4c_mark_failed("pcm_domain");

    p4c_signal_if_valid(p4c_pcm_failure_ready);
    p4c_signal_if_valid(p4c_pcm_done);
    ExitThread();
}

static int p4c_initialize_channels_and_sync(void)
{
    if (pstvnc_transport_rfb_channel_initialize(
            &p4c_rfb_channel,
            p4c_rfb_storage,
            sizeof(p4c_rfb_storage)) != 0)
        return 0;

    if (pstvnc_transport_audio_channel_initialize(
            &p4c_audio_channel,
            p4c_audio_storage,
            sizeof(p4c_audio_storage)) != 0)
        return 0;

    p4c_rfb_mutex = p4c_create_semaphore(1, 1);
    p4c_audio_mutex = p4c_create_semaphore(1, 1);
    p4c_audio_activity = p4c_create_semaphore(0, 1);

    p4c_pcm_start = p4c_create_semaphore(0, 1);
    p4c_pcm_failure_ready = p4c_create_semaphore(0, 1);

    p4c_rfb_first_gate = p4c_create_semaphore(0, 1);
    p4c_rfb_second_gate = p4c_create_semaphore(0, 1);

    p4c_outbound_slot = p4c_create_semaphore(1, 1);
    p4c_outbound_ready = p4c_create_semaphore(0, 1);
    p4c_outbound_done = p4c_create_semaphore(0, 1);

    p4c_io_done = p4c_create_semaphore(0, 1);
    p4c_rfb_done = p4c_create_semaphore(0, 1);
    p4c_pcm_done = p4c_create_semaphore(0, 1);

    return p4c_rfb_mutex >= 0 &&
        p4c_audio_mutex >= 0 &&
        p4c_audio_activity >= 0 &&
        p4c_pcm_start >= 0 &&
        p4c_pcm_failure_ready >= 0 &&
        p4c_rfb_first_gate >= 0 &&
        p4c_rfb_second_gate >= 0 &&
        p4c_outbound_slot >= 0 &&
        p4c_outbound_ready >= 0 &&
        p4c_outbound_done >= 0 &&
        p4c_io_done >= 0 &&
        p4c_rfb_done >= 0 &&
        p4c_pcm_done >= 0;
}

static void p4c_delete_semaphore_if_valid(int *semaphore_id)
{
    if (semaphore_id != NULL && *semaphore_id >= 0) {
        (void)DeleteSema(*semaphore_id);
        *semaphore_id = -1;
    }
}

static void p4c_release_sync(void)
{
    p4c_delete_semaphore_if_valid(&p4c_rfb_mutex);
    p4c_delete_semaphore_if_valid(&p4c_audio_mutex);
    p4c_delete_semaphore_if_valid(&p4c_audio_activity);
    p4c_delete_semaphore_if_valid(&p4c_pcm_start);
    p4c_delete_semaphore_if_valid(&p4c_pcm_failure_ready);
    p4c_delete_semaphore_if_valid(&p4c_rfb_first_gate);
    p4c_delete_semaphore_if_valid(&p4c_rfb_second_gate);
    p4c_delete_semaphore_if_valid(&p4c_outbound_slot);
    p4c_delete_semaphore_if_valid(&p4c_outbound_ready);
    p4c_delete_semaphore_if_valid(&p4c_outbound_done);
    p4c_delete_semaphore_if_valid(&p4c_io_done);
    p4c_delete_semaphore_if_valid(&p4c_rfb_done);
    p4c_delete_semaphore_if_valid(&p4c_pcm_done);
}

int main(int argc, char **argv)
{
    int socket_fd = -1;
    int success;

    (void)argc;
    (void)argv;

    memset(&p4c_stream, 0, sizeof(p4c_stream));
    memset(&p4c_rfb_channel, 0, sizeof(p4c_rfb_channel));
    memset(&p4c_audio_channel, 0, sizeof(p4c_audio_channel));
    memset(&p4c_outbound_work, 0, sizeof(p4c_outbound_work));

    init_scr();

    p4c_print("WIRE Q8/Q11 PROOF 4C");
    p4c_print("state=BOOT");
    p4c_print("PRIORITY_BASELINE=63/64/65/67");
    p4c_print("PRIORITIES_FINAL=NO");
    p4c_print("MPEG_PRIORITY=67 INACTIVE");

    if (pstvnc_ps2_system_prepare_iop() < 0) {
        p4c_print("FAIL prepare_iop");
        SleepThread();
        return 10;
    }

    p4c_print("state=NETWORK_INIT");

    if (pstvnc_ps2_network_init() < 0 ||
        pstvnc_ps2_network_wait_link() < 0) {
        p4c_print("FAIL network");
        SleepThread();
        return 11;
    }

    p4c_print("state=TCP_CONNECT");

    socket_fd = pstvnc_ps2_network_connect_pstv();
    if (socket_fd < 0) {
        p4c_print("FAIL tcp_connect");
        SleepThread();
        return 12;
    }

    if (!pstvnc_transport_physical_stream_adopt(
            &p4c_stream,
            socket_fd)) {
        pstvnc_ps2_network_close(socket_fd);
        p4c_print("FAIL stream_adopt");
        SleepThread();
        return 13;
    }

    socket_fd = -1;

    if (!p4c_initialize_channels_and_sync()) {
        (void)pstvnc_transport_physical_stream_release(&p4c_stream);
        p4c_print("FAIL sync_init");
        SleepThread();
        return 14;
    }

    p4c_rfb_thread_id = p4c_create_thread(
        p4c_rfb_thread,
        p4c_rfb_stack,
        sizeof(p4c_rfb_stack),
        P4C_RFB_PRIORITY);

    p4c_pcm_thread_id = p4c_create_thread(
        p4c_pcm_thread,
        p4c_pcm_stack,
        sizeof(p4c_pcm_stack),
        P4C_PCM_PRIORITY);

    p4c_io_thread_id = p4c_create_thread(
        p4c_io_thread,
        p4c_io_stack,
        sizeof(p4c_io_stack),
        P4C_TRANSPORT_IO_PRIORITY);

    if (p4c_rfb_thread_id < 0 ||
        p4c_pcm_thread_id < 0 ||
        p4c_io_thread_id < 0) {
        (void)pstvnc_transport_physical_stream_release(&p4c_stream);
        p4c_release_sync();
        p4c_print("FAIL thread_create");
        SleepThread();
        return 15;
    }

    /*
     * Domain workers start first and immediately block on proof-local readiness.
     * The priority-63 physical-I/O owner starts last and then owns all Wire I/O.
     */
    if (StartThread(p4c_rfb_thread_id, NULL) < 0 ||
        StartThread(p4c_pcm_thread_id, NULL) < 0 ||
        StartThread(p4c_io_thread_id, NULL) < 0) {
        p4c_mark_failed("thread_start");
        SleepThread();
        return 16;
    }

    if (WaitSema(p4c_io_done) < 0 ||
        WaitSema(p4c_rfb_done) < 0 ||
        WaitSema(p4c_pcm_done) < 0) {
        p4c_mark_failed("thread_completion");
        SleepThread();
        return 17;
    }

    success =
        !p4c_failed &&
        p4c_io_pass &&
        p4c_rfb_pass &&
        p4c_pcm_expected_failure &&
        p4c_rfb_full_during_audio_progress &&
        p4c_audio_full_after_failure;

    (void)DeleteThread(p4c_io_thread_id);
    (void)DeleteThread(p4c_rfb_thread_id);
    (void)DeleteThread(p4c_pcm_thread_id);

    p4c_io_thread_id = -1;
    p4c_rfb_thread_id = -1;
    p4c_pcm_thread_id = -1;

    p4c_release_sync();

    if (!success) {
        p4c_print("PROOF4C RIDER ISOLATION=FAIL");
        SleepThread();
        return 18;
    }

    p4c_print("RFB_THREAD_PRIORITY=64");
    p4c_print("PCM_THREAD_PRIORITY=65");
    p4c_print("P4C RIDER ISOLATION=PASS");
    p4c_print("PROOF4C RIDER ISOLATION COMPLETE");

    SleepThread();
    return 0;
}

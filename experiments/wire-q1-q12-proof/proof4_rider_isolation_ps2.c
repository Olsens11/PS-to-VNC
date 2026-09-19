/*
 * File synopsis:
 * Implements the PS2 half of Wire Proof 4: ordinary-rider isolation and
 * minimum-scope AUDIO failure containment.
 *
 * The proof deliberately uses the clean Transport runtime itself. Transport
 * adopts the physical socket before establishment, so HELLO/ACCEPT and all
 * later RFB/AUDIO traffic occupy one unbroken PSTV sequence space. The normal
 * sole receiver is started only after ACCEPT.
 *
 * Real mechanisms reused:
 *   - Transport physical stream and sole receiver runtime;
 *   - bounded RFB and AUDIO channels with independent credits;
 *   - RFB's public bridge;
 *   - the synchronous PCM playback core.
 *
 * Development apparatus:
 *   - establishment through the proof-local Q4 representation;
 *   - public-bridge adapters that bind the proof's local Transport runtime;
 *   - an injected AUDIO service whose play operation fails deterministically;
 *   - fixed proof capacities/payloads and foreground Pi peer.
 *
 * The injected service failure is intentionally above Transport. It tests that
 * a PCM-domain failure can complete its local cleanup while the Wire and RFB
 * sibling remain healthy. It does not corrupt PSTV framing or Transport state.
 */

#include <debug.h>
#include <kernel.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "audio/playback.h"
#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "rfb/bridge.h"
#include "transport/bridge.h"
#include "transport/physical_stream.h"
#include "transport/protocol.h"
#include "transport/runtime.h"
#include "transport/transport.h"

#include "wire_establish_protocol.h"

#define PROOF4_RFB_CAPACITY 32u
#define PROOF4_AUDIO_CAPACITY 16u
#define PROOF4_MAX_DATA_PAYLOAD 16u
#define PROOF4_AUDIO_TRIGGER_BYTES 4u
#define PROOF4_OUTBOUND_RFB_BYTES 37u
#define PROOF4_WAIT_STEP_US 1000u
#define PROOF4_WAIT_LIMIT 10000u

static pstvnc_transport_runtime_t proof_runtime;
static int proof_runtime_active;

static const uint8_t expected_rfb_first[PROOF4_RFB_CAPACITY] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

static const uint8_t expected_rfb_second[PROOF4_RFB_CAPACITY] = {
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f
};

static const uint8_t expected_audio_trigger[PROOF4_AUDIO_TRIGGER_BYTES] = {
    0xa1, 0xa2, 0xa3, 0xa4
};

static const uint8_t expected_outbound_rfb[PROOF4_OUTBOUND_RFB_BYTES] = {
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4
};

typedef struct proof_audio_service_state {
    int initialize_calls;
    int format_calls;
    int volume_calls;
    int wait_calls;
    int play_calls;
    int stop_calls;
    int trigger_bytes_match;
} proof_audio_service_state_t;

static proof_audio_service_state_t proof_audio_service;

static void proof_print(const char *message)
{
    printf("%s\n", message);
    scr_printf("%s\n", message);
}

static pstvnc_transport_result_t proof_transport_terminal_result(void)
{
    if (!proof_runtime_active)
        return PSTVNC_TRANSPORT_INVALID;
    if (proof_runtime.stop_requested)
        return PSTVNC_TRANSPORT_STOPPED;
    if (proof_runtime.failed)
        return PSTVNC_TRANSPORT_FAILED;
    if (proof_runtime.receiver_done)
        return PSTVNC_TRANSPORT_CLOSED;

    return PSTVNC_TRANSPORT_FAILED;
}

/*
 * Proof-local public Transport bridge adapters.
 *
 * They do not implement queue, framing, credit, receiver, or socket mechanics;
 * those remain in the real clean Transport runtime. These adapters only bind
 * domain-facing public seams to this proof's explicitly owned runtime instance.
 */

pstvnc_transport_result_t pstvnc_transport_rfb_read_exact(
    void *buffer,
    size_t count)
{
    if (!proof_runtime_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_rfb_read_exact(
            &proof_runtime,
            buffer,
            count))
        return PSTVNC_TRANSPORT_OK;

    return proof_transport_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_poll_receive(void)
{
    int result;

    if (!proof_runtime_active)
        return PSTVNC_TRANSPORT_INVALID;

    result = pstvnc_transport_runtime_rfb_poll_receive(&proof_runtime);

    if (result > 0)
        return PSTVNC_TRANSPORT_OK;
    if (result == 0 &&
        !proof_runtime.failed &&
        !proof_runtime.receiver_done &&
        !proof_runtime.stop_requested)
        return PSTVNC_TRANSPORT_WOULD_BLOCK;

    return proof_transport_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_rfb_write_exact(
    const void *buffer,
    size_t count)
{
    if (!proof_runtime_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_rfb_write_exact(
            &proof_runtime,
            buffer,
            count))
        return PSTVNC_TRANSPORT_OK;

    return proof_transport_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_audio_read_available(
    void *buffer,
    size_t maximum_count,
    size_t *read_count)
{
    if (!proof_runtime_active)
        return PSTVNC_TRANSPORT_INVALID;

    return pstvnc_transport_runtime_audio_read_available(
        &proof_runtime,
        buffer,
        maximum_count,
        read_count);
}

pstvnc_transport_result_t pstvnc_transport_audio_status(
    size_t *available_count,
    int *producer_done)
{
    if (!proof_runtime_active)
        return PSTVNC_TRANSPORT_INVALID;

    return pstvnc_transport_runtime_audio_status(
        &proof_runtime,
        available_count,
        producer_done);
}

pstvnc_transport_result_t pstvnc_transport_audio_activity_snapshot(
    uint32_t *activity_sequence)
{
    if (!proof_runtime_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_audio_activity_snapshot(
            &proof_runtime,
            activity_sequence))
        return PSTVNC_TRANSPORT_OK;

    return proof_transport_terminal_result();
}

pstvnc_transport_result_t pstvnc_transport_audio_wait_activity(
    uint32_t *activity_sequence)
{
    if (!proof_runtime_active)
        return PSTVNC_TRANSPORT_INVALID;

    if (pstvnc_transport_runtime_audio_wait_activity(
            &proof_runtime,
            activity_sequence))
        return PSTVNC_TRANSPORT_OK;

    return proof_transport_terminal_result();
}

/*
 * RFB quiescence is outside this proof's bounded rider-isolation transaction.
 * These symbols satisfy the complete real RFB public bridge surface without
 * inventing a fake successful quiesce result.
 */

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

static int proof_audio_initialize(void *context)
{
    proof_audio_service_state_t *state =
        (proof_audio_service_state_t *)context;

    state->initialize_calls++;
    return 0;
}

static int proof_audio_set_format(
    void *context,
    uint32_t rate_hz,
    uint32_t channels,
    uint32_t bits_per_sample)
{
    proof_audio_service_state_t *state =
        (proof_audio_service_state_t *)context;

    state->format_calls++;

    return rate_hz == 44100u &&
        channels == 2u &&
        bits_per_sample == 16u
        ? 0
        : -1;
}

static int proof_audio_set_volume(
    void *context,
    uint32_t volume_percent)
{
    proof_audio_service_state_t *state =
        (proof_audio_service_state_t *)context;

    state->volume_calls++;
    return volume_percent == 73u ? 0 : -1;
}

static int proof_audio_wait_audio(
    void *context,
    size_t byte_count)
{
    proof_audio_service_state_t *state =
        (proof_audio_service_state_t *)context;

    state->wait_calls++;
    return byte_count == PROOF4_AUDIO_TRIGGER_BYTES ? 0 : -1;
}

static int proof_audio_play_audio(
    void *context,
    const uint8_t *bytes,
    size_t byte_count)
{
    proof_audio_service_state_t *state =
        (proof_audio_service_state_t *)context;

    state->play_calls++;

    if (bytes != NULL &&
        byte_count == sizeof(expected_audio_trigger) &&
        memcmp(
            bytes,
            expected_audio_trigger,
            sizeof(expected_audio_trigger)) == 0)
        state->trigger_bytes_match = 1;

    /*
     * Deliberate domain failure. The bytes were successfully delivered across
     * the AUDIO public Transport seam; only the playback service operation fails.
     */
    return -1;
}

static int proof_audio_stop_audio(void *context)
{
    proof_audio_service_state_t *state =
        (proof_audio_service_state_t *)context;

    state->stop_calls++;
    return 0;
}

static int establish_wire(uint32_t *session_id)
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

    /*
     * The real Transport runtime already owns the physical stream at this
     * point, but its receiver thread is not started. HELLO therefore uses its
     * real send sequence 1.
     */
    if (!pstvnc_transport_physical_stream_send_frame(
            &proof_runtime.physical_stream,
            PSTVNC_TRANSPORT_FRAME_HELLO,
            PSTVNC_TRANSPORT_CHANNEL_CONTROL,
            0u,
            hello_payload,
            sizeof(hello_payload)))
        return 0;

    if (!pstvnc_transport_physical_stream_receive_frame(
            &proof_runtime.physical_stream,
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

static int proof_send_failure_marker(void)
{
    /*
     * Outbound CONTROL heartbeat is proof apparatus only. It gives the Pi a
     * machine-visible ordering point after the local AUDIO failure has completed
     * and after the failed AUDIO rider's Relay has been observed full.
     *
     * The real Transport physical stream still owns sequence and serialization.
     */
    return pstvnc_transport_physical_stream_send_frame(
        &proof_runtime.physical_stream,
        PSTVNC_TRANSPORT_FRAME_HEARTBEAT,
        PSTVNC_TRANSPORT_CHANNEL_CONTROL,
        0u,
        NULL,
        0u);
}

static int wait_for_audio_full(void)
{
    uint32_t attempt;

    for (attempt = 0u; attempt < PROOF4_WAIT_LIMIT; attempt++) {
        size_t available = 0u;
        int producer_done = 0;
        pstvnc_transport_result_t result;

        result = pstvnc_transport_runtime_audio_status(
            &proof_runtime,
            &available,
            &producer_done);

        if (result != PSTVNC_TRANSPORT_OK)
            return 0;

        if (producer_done)
            return 0;

        if (available == PROOF4_AUDIO_CAPACITY)
            return 1;

        if (available > PROOF4_AUDIO_CAPACITY)
            return 0;

        if (pstvnc_ps2_system_delay_us(PROOF4_WAIT_STEP_US) < 0)
            return 0;
    }

    return 0;
}

static void proof_shutdown_runtime(void)
{
    if (!proof_runtime_active)
        return;

    if (proof_runtime.receiver_thread_started &&
        !proof_runtime.receiver_done) {
        (void)pstvnc_transport_runtime_request_stop(&proof_runtime);
        (void)pstvnc_transport_runtime_wait_receiver_done(&proof_runtime);
    }

    (void)pstvnc_transport_runtime_release(&proof_runtime);
    proof_runtime_active = 0;
}

int main(int argc, char **argv)
{
    int socket_fd = -1;
    int establish_result;
    uint32_t session_id = 0u;
    pstvnc_transport_session_config_t transport_config;
    pstvnc_transport_audio_channel_config_t audio_config;
    pstvnc_config_pcm_profile_t pcm_profile;
    pstvnc_audio_service_ops_t audio_service;
    pstvnc_audio_playback_report_t playback_report;
    pstvnc_audio_playback_result_t playback_result;
    uint8_t audio_buffer[PROOF4_AUDIO_TRIGGER_BYTES];
    uint8_t first_rfb[PROOF4_RFB_CAPACITY];
    uint8_t second_rfb[PROOF4_RFB_CAPACITY];
    char line[128];

    (void)argc;
    (void)argv;

    memset(&proof_runtime, 0, sizeof(proof_runtime));
    memset(&proof_audio_service, 0, sizeof(proof_audio_service));
    memset(&transport_config, 0, sizeof(transport_config));
    memset(&audio_config, 0, sizeof(audio_config));
    memset(&pcm_profile, 0, sizeof(pcm_profile));
    memset(&audio_service, 0, sizeof(audio_service));
    memset(&playback_report, 0, sizeof(playback_report));

    init_scr();

    proof_print("WIRE Q8/Q11 PROOF 4");
    proof_print("state=BOOT");

    if (pstvnc_ps2_system_prepare_iop() < 0) {
        proof_print("FAIL prepare_iop");
        SleepThread();
        return 10;
    }

    proof_print("state=NETWORK_INIT");

    if (pstvnc_ps2_network_init() < 0 ||
        pstvnc_ps2_network_wait_link() < 0) {
        proof_print("FAIL network");
        SleepThread();
        return 11;
    }

    proof_print("state=TCP_CONNECT");

    socket_fd = pstvnc_ps2_network_connect_pstv();
    if (socket_fd < 0) {
        proof_print("FAIL tcp_connect");
        SleepThread();
        return 12;
    }

    transport_config.rfb_queue_capacity = PROOF4_RFB_CAPACITY;
    transport_config.rfb_initial_credit_bytes = PROOF4_RFB_CAPACITY;
    transport_config.rfb_credit_batch_bytes = PROOF4_RFB_CAPACITY;
    transport_config.rfb_credit_flush_on_empty = 1;
    transport_config.rfb_credit_return_enabled = 1;
    transport_config.receiver_thread_stack_size = 16384u;
    transport_config.receiver_thread_priority = 64;
    transport_config.max_data_payload = PROOF4_MAX_DATA_PAYLOAD;

    audio_config.queue_capacity = PROOF4_AUDIO_CAPACITY;
    audio_config.initial_credit_bytes = PROOF4_AUDIO_CAPACITY;
    audio_config.credit_batch_bytes = PROOF4_AUDIO_TRIGGER_BYTES;
    audio_config.credit_flush_on_empty = 1;
    audio_config.credit_return_enabled = 1;

    /*
     * Real Transport adopts the physical stream before Wire byte one.
     * start_receiver() is intentionally deferred until establishment succeeds.
     */
    if (!pstvnc_transport_runtime_initialize_with_audio(
            &proof_runtime,
            socket_fd,
            &transport_config,
            &audio_config)) {
        pstvnc_ps2_network_close(socket_fd);
        proof_print("FAIL transport_init");
        SleepThread();
        return 13;
    }

    proof_runtime_active = 1;
    socket_fd = -1;

    proof_print("state=PROVISIONAL");

    establish_result = establish_wire(&session_id);
    if (establish_result <= 0) {
        snprintf(
            line,
            sizeof(line),
            "FAIL establishment result=%d",
            establish_result);
        proof_print(line);
        proof_shutdown_runtime();
        SleepThread();
        return 14;
    }

    snprintf(
        line,
        sizeof(line),
        "WIRE ACTIVE id=%u",
        (unsigned int)session_id);
    proof_print(line);

    /*
     * Sequence state already advanced through HELLO/ACCEPT. Starting the real
     * receiver therefore sends its RFB/AUDIO initial credits as outbound
     * sequences 2 and 3 and receives ordinary rider DATA from inbound sequence 2.
     */
    if (!pstvnc_transport_runtime_start_receiver(&proof_runtime)) {
        proof_print("FAIL receiver_start");
        proof_shutdown_runtime();
        SleepThread();
        return 15;
    }

    proof_print("RFB_CREDIT=32 AUDIO_CREDIT=16");
    proof_print("state=RFB_STALLED_FULL");

    pcm_profile.rate_hz = 44100u;
    pcm_profile.channels = 2u;
    pcm_profile.bits_per_sample = 16u;
    pcm_profile.volume_percent = 73u;

    audio_service.initialize = proof_audio_initialize;
    audio_service.set_format = proof_audio_set_format;
    audio_service.set_volume = proof_audio_set_volume;
    audio_service.wait_audio = proof_audio_wait_audio;
    audio_service.play_audio = proof_audio_play_audio;
    audio_service.stop_audio = proof_audio_stop_audio;
    audio_service.context = &proof_audio_service;

    /*
     * Pi first fills RFB, then sends AUDIO. The real sole receiver must dispatch
     * AUDIO even though the sibling RFB Relay has no remaining capacity.
     */
    playback_result = pstvnc_audio_playback_run(
        &pcm_profile,
        audio_buffer,
        sizeof(audio_buffer),
        &audio_service,
        &playback_report);

    if (playback_result != PSTVNC_AUDIO_PLAYBACK_SERVICE_PLAY_FAILED) {
        snprintf(
            line,
            sizeof(line),
            "FAIL audio_result=%d",
            (int)playback_result);
        proof_print(line);
        proof_shutdown_runtime();
        SleepThread();
        return 16;
    }

    if (proof_audio_service.initialize_calls != 1 ||
        proof_audio_service.format_calls != 1 ||
        proof_audio_service.volume_calls != 1 ||
        proof_audio_service.wait_calls != 1 ||
        proof_audio_service.play_calls != 1 ||
        proof_audio_service.stop_calls != 1 ||
        !proof_audio_service.trigger_bytes_match ||
        playback_report.submitted_bytes != 0u ||
        playback_report.submitted_chunks != 0u ||
        playback_report.cleanup_attempted != 1 ||
        playback_report.cleanup_failed != 0) {
        proof_print("FAIL audio_failure_contract");
        proof_shutdown_runtime();
        SleepThread();
        return 17;
    }

    if (proof_runtime.failed) {
        proof_print("FAIL transport_failed_with_audio");
        proof_shutdown_runtime();
        SleepThread();
        return 18;
    }

    proof_print("AUDIO_PROGRESS_WHILE_RFB_FULL=YES");
    proof_print("AUDIO_FAILURE=SERVICE_PLAY");
    proof_print("AUDIO_CLEANUP=PASS");
    proof_print("AUDIO_FAILURE_LOCAL=YES");
    proof_print("WIRE_AFTER_AUDIO_FAILURE=ACTIVE");

    /*
     * After the playback owner has failed and returned, Pi uses the available
     * AUDIO credit to fill channel 2 completely. No AUDIO domain consumer runs
     * after this point.
     */
    if (!wait_for_audio_full()) {
        proof_print("FAIL audio_not_full");
        proof_shutdown_runtime();
        SleepThread();
        return 19;
    }

    proof_print("AUDIO_RELAY_FULL_AFTER_FAILURE=16");

    /*
     * This machine-visible marker is sent only after both facts above are true:
     * the audio-domain failure completed locally, and its Relay is now full.
     */
    if (!proof_send_failure_marker()) {
        proof_print("FAIL failure_marker");
        proof_shutdown_runtime();
        SleepThread();
        return 20;
    }

    proof_print("AUDIO_FAILURE_MARKER_SENT=YES");

    /*
     * RFB was filled before AUDIO ever ran. Draining it now proves the sibling
     * can resume after AUDIO failed while AUDIO's own Relay remains full.
     */
    if (pstvnc_rfb_bridge_read_exact(
            first_rfb,
            sizeof(first_rfb)) != 0 ||
        memcmp(
            first_rfb,
            expected_rfb_first,
            sizeof(first_rfb)) != 0) {
        proof_print("FAIL rfb_first");
        proof_shutdown_runtime();
        SleepThread();
        return 21;
    }

    proof_print("RFB_FIRST_DRAIN_AFTER_AUDIO_FAIL=32");

    /*
     * Pi sends an entirely new second RFB window only after seeing the first
     * drain credit. The real sole receiver must accept it while AUDIO remains
     * failed and its Relay remains full.
     */
    if (pstvnc_rfb_bridge_read_exact(
            second_rfb,
            sizeof(second_rfb)) != 0 ||
        memcmp(
            second_rfb,
            expected_rfb_second,
            sizeof(second_rfb)) != 0) {
        proof_print("FAIL rfb_second");
        proof_shutdown_runtime();
        SleepThread();
        return 22;
    }

    proof_print("RFB_PROGRESS_WITH_FAILED_AUDIO=YES");

    if (proof_runtime.failed) {
        proof_print("FAIL transport_failed_after_rfb");
        proof_shutdown_runtime();
        SleepThread();
        return 23;
    }

    if (pstvnc_rfb_bridge_write_exact(
            expected_outbound_rfb,
            sizeof(expected_outbound_rfb)) != 0) {
        proof_print("FAIL rfb_outbound");
        proof_shutdown_runtime();
        SleepThread();
        return 24;
    }

    proof_print("RFB_OUTBOUND_AFTER_AUDIO_FAIL=37");

    if (proof_runtime.failed) {
        proof_print("FAIL transport_final");
        proof_shutdown_runtime();
        SleepThread();
        return 25;
    }

    proof_print("TRANSPORT_FAILED=NO");
    proof_print("WIRE_HEALTHY_WITH_AUDIO_FAILED=YES");

    if (!pstvnc_transport_runtime_request_stop(&proof_runtime)) {
        proof_print("FAIL transport_stop");
        proof_shutdown_runtime();
        SleepThread();
        return 26;
    }

    if (!pstvnc_transport_runtime_wait_receiver_done(&proof_runtime)) {
        proof_print("FAIL receiver_join");
        proof_shutdown_runtime();
        SleepThread();
        return 27;
    }

    if (!pstvnc_transport_runtime_release(&proof_runtime)) {
        proof_print("FAIL transport_release");
        proof_runtime_active = 0;
        SleepThread();
        return 28;
    }

    proof_runtime_active = 0;

    proof_print("WIRE INACTIVE");
    proof_print("PROOF4 RIDER ISOLATION COMPLETE");

    SleepThread();
    return 0;
}

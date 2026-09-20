/*
 * File synopsis:
 * Proof 6: one real MPEG generation inside one live Wire Session.
 *
 * The proof links current product Transport and src/mpeg/decoder.c unchanged.
 * Q4 establishment and the compact RFB-channel control markers are proof-local
 * apparatus only.
 *
 * The decisive stop discriminator is asynchronous: the main EE thread requests
 * decoder stop while the priority-67 MPEG worker is inside a real MPEG_Picture()
 * call. The clean decoder feed callback must continue supplying real Wire MPEG
 * bytes until that decoder call returns; only then may STOPPED be observed.
 *
 * After decoder release the Pi stops exact proof generation production and
 * acknowledges that no more MPEG DATA will be sent. The PS2 then publishes
 * producer completion, drains/discards already-accepted residual MPEG bytes,
 * and proves ordinary logical-RFB traffic still works on the same Wire Session.
 */

#include <debug.h>
#include <kernel.h>
#include <malloc.h>

#include <sys/socket.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpeg/decoder.h"
#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "transport/bridge.h"
#include "transport/protocol.h"
#include "transport/transport.h"

#include "proof6_mpeg_platform_ps2.h"
#include "wire_establish_protocol.h"

#define PROOF6_MPEG_QUEUE_BYTES (512u * 1024u)
#define PROOF6_MPEG_INITIAL_CREDIT_BYTES (512u * 1024u)
#define PROOF6_MPEG_CREDIT_BATCH_BYTES 8192u
#define PROOF6_MPEG_FEED_BYTES 2048u
#define PROOF6_MPEG_WORKER_STACK_BYTES (64u * 1024u)
#define PROOF6_MPEG_WORKER_PRIORITY 67
#define PROOF6_STOP_ARM_PICTURES 90u
#define PROOF6_STOP_WAIT_LOOPS 15000u
#define PROOF6_JOIN_WAIT_LOOPS 5000u
#define PROOF6_RFB_WAIT_LOOPS 10000u
#define PROOF6_POLL_DELAY_US 1000u

#define PROOF6_STATUS_BYTES 20u
#define PROOF6_LIVE_BYTES 12u
#define PROOF6_ACK_BYTES 4u

typedef struct proof6_sync_context {
    int semaphore_id;
} proof6_sync_context_t;

typedef struct proof6_worker {
    pstvnc_mpeg_decoder_t decoder;
    pstvnc_mpeg_decoder_report_t report;
    proof6_mpeg_platform_t platform;
    proof6_sync_context_t sync;

    void *thread_stack_allocation;
    void *thread_stack;
    int thread_id;
    int thread_started;

    volatile int decoder_ready;
    volatile int finished;

    pstvnc_mpeg_decoder_result_t initialize_result;
    pstvnc_mpeg_decoder_result_t run_result;
    pstvnc_mpeg_decoder_result_t release_result;
} proof6_worker_t;

static void proof6_print(const char *message)
{
    printf("%s\n", message);
    scr_printf("%s\n", message);
}

static int proof6_send_exact(int socket_fd, const void *buffer, size_t count)
{
    const uint8_t *bytes = (const uint8_t *)buffer;
    size_t sent = 0u;

    while (sent < count) {
        int result = send(socket_fd, bytes + sent, count - sent, 0);

        if (result <= 0)
            return 0;

        sent += (size_t)result;
    }

    return 1;
}

static int proof6_receive_exact(int socket_fd, void *buffer, size_t count)
{
    uint8_t *bytes = (uint8_t *)buffer;
    size_t received = 0u;

    while (received < count) {
        int result = recv(socket_fd, bytes + received, count - received, 0);

        if (result <= 0)
            return 0;

        received += (size_t)result;
    }

    return 1;
}

static int proof6_establish_session(
    int *socket_fd,
    uint32_t *session_id)
{
    uint8_t wire_header[PSTVNC_TRANSPORT_HEADER_SIZE];
    uint8_t hello_payload[PSTVNC_WIRE_PROOF_HELLO_BYTES];
    uint8_t result_payload[PSTVNC_WIRE_PROOF_ACCEPT_BYTES];
    pstvnc_transport_header_t header;
    pstvnc_wire_proof_hello_t hello;
    pstvnc_wire_proof_accept_t acceptance;

    if (socket_fd == NULL || session_id == NULL)
        return 0;

    *session_id = 0u;
    *socket_fd = pstvnc_ps2_network_connect_pstv();
    if (*socket_fd < 0)
        return 0;

    hello.wire_version = PSTVNC_WIRE_PROOF_WIRE_VERSION;
    hello.product_version = PSTVNC_WIRE_PROOF_PRODUCT_VERSION;

    if (!pstvnc_wire_proof_hello_encode(hello_payload, &hello))
        goto fail;

    memset(&header, 0, sizeof(header));
    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = PSTVNC_TRANSPORT_FRAME_HELLO;
    header.channel = PSTVNC_TRANSPORT_CHANNEL_CONTROL;
    header.sequence = 1u;
    header.payload_length = sizeof(hello_payload);

    if (!pstvnc_transport_header_encode(wire_header, &header) ||
        !proof6_send_exact(*socket_fd, wire_header, sizeof(wire_header)) ||
        !proof6_send_exact(*socket_fd, hello_payload, sizeof(hello_payload)))
        goto fail;

    if (!proof6_receive_exact(*socket_fd, wire_header, sizeof(wire_header)) ||
        !pstvnc_transport_header_decode(&header, wire_header) ||
        header.kind != PSTVNC_WIRE_PROOF_FRAME_ACCEPT ||
        header.channel != PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
        header.flags != 0u ||
        header.sequence != 1u ||
        header.payload_length != sizeof(result_payload) ||
        !proof6_receive_exact(*socket_fd, result_payload, sizeof(result_payload)) ||
        !pstvnc_wire_proof_accept_decode(
            &acceptance,
            result_payload,
            sizeof(result_payload)) ||
        acceptance.session_id == 0u)
        goto fail;

    *session_id = acceptance.session_id;
    return 1;

fail:
    pstvnc_ps2_network_close(*socket_fd);
    *socket_fd = -1;
    return 0;
}

static pstvnc_transport_session_config_t proof6_make_transport_config(void)
{
    pstvnc_transport_session_config_t config;

    memset(&config, 0, sizeof(config));
    config.rfb_queue_capacity = 64u;
    config.rfb_initial_credit_bytes = 64u;
    config.rfb_credit_batch_bytes = 4u;
    config.rfb_credit_flush_on_empty = 1;
    config.rfb_credit_return_enabled = 1;
    config.receiver_thread_stack_size = 16384u;
    config.receiver_thread_priority = 63;
    config.max_data_payload = PSTVNC_TRANSPORT_MAX_PAYLOAD;
    return config;
}

static pstvnc_transport_mpeg_channel_config_t proof6_make_mpeg_config(void)
{
    pstvnc_transport_mpeg_channel_config_t config;

    memset(&config, 0, sizeof(config));
    config.queue_capacity = PROOF6_MPEG_QUEUE_BYTES;
    config.initial_credit_bytes = PROOF6_MPEG_INITIAL_CREDIT_BYTES;
    config.credit_batch_bytes = PROOF6_MPEG_CREDIT_BATCH_BYTES;
    config.credit_flush_on_empty = 1;
    config.credit_return_enabled = 1;
    return config;
}

static void *proof6_memory_allocate(
    void *context,
    size_t byte_count,
    size_t alignment)
{
    (void)context;
    return memalign(alignment, byte_count);
}

static void proof6_memory_release(void *context, void *pointer)
{
    (void)context;
    free(pointer);
}

static int proof6_sync_lock(void *context)
{
    proof6_sync_context_t *sync =
        (proof6_sync_context_t *)context;

    return sync != NULL && sync->semaphore_id >= 0 &&
        WaitSema(sync->semaphore_id) >= 0
        ? 0 : -1;
}

static int proof6_sync_unlock(void *context)
{
    proof6_sync_context_t *sync =
        (proof6_sync_context_t *)context;

    return sync != NULL && sync->semaphore_id >= 0 &&
        SignalSema(sync->semaphore_id) >= 0
        ? 0 : -1;
}

static int proof6_create_mutex_semaphore(void)
{
    ee_sema_t semaphore;

    memset(&semaphore, 0, sizeof(semaphore));
    semaphore.init_count = 1;
    semaphore.max_count = 1;
    semaphore.option = 0;
    return CreateSema(&semaphore);
}

static void *proof6_align64(void *allocation)
{
    uintptr_t address = (uintptr_t)allocation;
    address = (address + 63u) & ~(uintptr_t)63u;
    return (void *)address;
}

static void proof6_worker_thread(void *argument)
{
    proof6_worker_t *worker = (proof6_worker_t *)argument;
    pstvnc_mpeg_decoder_config_t config;
    pstvnc_mpeg_decoder_memory_ops_t memory_ops;
    pstvnc_mpeg_decoder_sync_ops_t sync_ops;
    pstvnc_mpeg_decoder_platform_ops_t platform_ops;

    memset(&config, 0, sizeof(config));
    config.max_width = 704u;
    config.max_height = 480u;
    config.bytes_per_pixel = 4u;
    config.feed_payload_capacity = PROOF6_MPEG_FEED_BYTES;
    config.transfer_alignment = 16u;
    config.buffer_alignment = 64u;

    memset(&memory_ops, 0, sizeof(memory_ops));
    memory_ops.allocate = proof6_memory_allocate;
    memory_ops.release = proof6_memory_release;

    memset(&sync_ops, 0, sizeof(sync_ops));
    sync_ops.lock = proof6_sync_lock;
    sync_ops.unlock = proof6_sync_unlock;
    sync_ops.context = &worker->sync;

    proof6_mpeg_platform_make_ops(&worker->platform, &platform_ops);

    worker->initialize_result = pstvnc_mpeg_decoder_initialize(
        &worker->decoder,
        &config,
        &memory_ops,
        &sync_ops,
        &platform_ops);

    if (worker->initialize_result == PSTVNC_MPEG_DECODER_COMPLETE) {
        worker->decoder_ready = 1;
        worker->run_result = pstvnc_mpeg_decoder_run(
            &worker->decoder,
            &worker->report);
        worker->decoder_ready = 0;
        worker->release_result =
            pstvnc_mpeg_decoder_release(&worker->decoder);
    } else {
        worker->run_result = worker->initialize_result;
        worker->release_result =
            pstvnc_mpeg_decoder_release(&worker->decoder);
    }

    worker->finished = 1;
    ExitThread();
}

static int proof6_worker_start(proof6_worker_t *worker)
{
    ee_thread_t thread;

    if (worker == NULL)
        return 0;

    memset(worker, 0, sizeof(*worker));
    worker->thread_id = -1;
    worker->sync.semaphore_id = -1;

    worker->sync.semaphore_id = proof6_create_mutex_semaphore();
    if (worker->sync.semaphore_id < 0)
        return 0;

    if (!proof6_mpeg_platform_context_initialize(
            &worker->platform,
            &worker->decoder))
        return 0;

    worker->thread_stack_allocation =
        malloc(PROOF6_MPEG_WORKER_STACK_BYTES + 63u);
    if (worker->thread_stack_allocation == NULL)
        return 0;

    worker->thread_stack =
        proof6_align64(worker->thread_stack_allocation);

    memset(&thread, 0, sizeof(thread));
    thread.func = proof6_worker_thread;
    thread.stack = worker->thread_stack;
    thread.stack_size = PROOF6_MPEG_WORKER_STACK_BYTES;
    thread.gp_reg = &_gp;
    thread.initial_priority = PROOF6_MPEG_WORKER_PRIORITY;

    worker->thread_id = CreateThread(&thread);
    if (worker->thread_id < 0)
        return 0;

    if (StartThread(worker->thread_id, worker) < 0) {
        (void)DeleteThread(worker->thread_id);
        worker->thread_id = -1;
        return 0;
    }

    worker->thread_started = 1;
    return 1;
}

static int proof6_worker_join(proof6_worker_t *worker)
{
    unsigned int loops = 0u;

    if (worker == NULL || !worker->thread_started)
        return 0;

    while (!worker->finished && loops < PROOF6_JOIN_WAIT_LOOPS) {
        if (pstvnc_ps2_system_delay_us(PROOF6_POLL_DELAY_US) < 0)
            return 0;
        loops++;
    }

    if (!worker->finished)
        return 0;

    for (loops = 0u; loops < PROOF6_JOIN_WAIT_LOOPS; loops++) {
        ee_thread_status_t status;

        memset(&status, 0, sizeof(status));
        if (ReferThreadStatus(worker->thread_id, &status) < 0)
            return 0;

        if (status.status == THS_DORMANT)
            break;

        if (pstvnc_ps2_system_delay_us(PROOF6_POLL_DELAY_US) < 0)
            return 0;
    }

    if (loops == PROOF6_JOIN_WAIT_LOOPS)
        return 0;

    if (DeleteThread(worker->thread_id) < 0)
        return 0;

    worker->thread_started = 0;
    worker->thread_id = -1;
    free(worker->thread_stack_allocation);
    worker->thread_stack_allocation = NULL;
    worker->thread_stack = NULL;
    return 1;
}

static int proof6_wait_for_rfb(
    const pstvnc_transport_access_t *transport_access)
{
    unsigned int loops;

    for (loops = 0u; loops < PROOF6_RFB_WAIT_LOOPS; loops++) {
        pstvnc_transport_result_t result =
            pstvnc_transport_rfb_poll_receive(transport_access);

        if (result == PSTVNC_TRANSPORT_OK)
            return 1;

        if (result != PSTVNC_TRANSPORT_WOULD_BLOCK)
            return 0;

        if (pstvnc_ps2_system_delay_us(PROOF6_POLL_DELAY_US) < 0)
            return 0;
    }

    return 0;
}

static int proof6_read_rfb_exact(
    const pstvnc_transport_access_t *transport_access,
    uint8_t output[PROOF6_ACK_BYTES])
{
    if (!proof6_wait_for_rfb(transport_access))
        return 0;

    return pstvnc_transport_rfb_read_exact(
        transport_access,
        output,
        PROOF6_ACK_BYTES) == PSTVNC_TRANSPORT_OK;
}

static int proof6_drain_mpeg_residual(
    const pstvnc_transport_access_t *transport_access,
    uint32_t *discarded_bytes)
{
    uint8_t *buffer;
    uint64_t total = 0u;

    if (discarded_bytes == NULL)
        return 0;

    *discarded_bytes = 0u;
    buffer = (uint8_t *)malloc(PSTVNC_TRANSPORT_MAX_PAYLOAD);
    if (buffer == NULL)
        return 0;

    for (;;) {
        size_t read_count = 0u;
        pstvnc_transport_result_t result =
            pstvnc_transport_mpeg_read_available(
                transport_access,
                buffer,
                PSTVNC_TRANSPORT_MAX_PAYLOAD,
                &read_count);

        if (result == PSTVNC_TRANSPORT_OK) {
            total += read_count;
            if (total > UINT32_MAX) {
                free(buffer);
                return 0;
            }
            continue;
        }

        if (result == PSTVNC_TRANSPORT_EXHAUSTED)
            break;

        free(buffer);
        return 0;
    }

    free(buffer);
    *discarded_bytes = (uint32_t)total;
    return 1;
}

static int proof6_retire_closed_session(void)
{
    if (pstvnc_transport_session_wait_receiver_done() != PSTVNC_TRANSPORT_OK)
        return 0;

    return pstvnc_transport_session_close() == PSTVNC_TRANSPORT_OK;
}

int main(int argc, char **argv)
{
    pstvnc_transport_session_config_t transport_config =
        proof6_make_transport_config();
    pstvnc_transport_mpeg_channel_config_t mpeg_config =
        proof6_make_mpeg_config();
    pstvnc_transport_access_t control_access;
    proof6_worker_t worker;
    uint8_t stop_status[PROOF6_STATUS_BYTES];
    uint8_t live_status[PROOF6_LIVE_BYTES];
    uint8_t ack[PROOF6_ACK_BYTES];
    uint32_t proof_session_id = 0u;
    uint32_t residual_discarded = 0u;
    uint32_t stop_requested_while_picture = 0u;
    uint32_t payload_bytes_low = 0u;
    unsigned int loops;
    int socket_fd = -1;
    int transport_open = 0;
    int stop_requested = 0;
    int pass = 0;

    (void)argc;
    (void)argv;

    memset(&control_access, 0, sizeof(control_access));
    memset(&worker, 0, sizeof(worker));
    worker.thread_id = -1;
    worker.sync.semaphore_id = -1;

    init_scr();
    proof6_print("WIRE PROOF6 MPEG GENERATION");
    proof6_print("Q4_ESTABLISHMENT=PROOF_LOCAL");
    proof6_print("TRANSPORT=PRODUCT_SOURCE");
    proof6_print("MPEG_DECODER=CLEAN_PRODUCT_SOURCE");
    proof6_print("RFB_CONTROL_MARKERS=PROOF_LOCAL");

    if (pstvnc_ps2_system_prepare_iop() < 0) {
        proof6_print("FAIL prepare_iop");
        goto done;
    }

    if (pstvnc_ps2_network_init() < 0 ||
        pstvnc_ps2_network_wait_link() < 0) {
        proof6_print("FAIL network");
        goto done;
    }

    if (!proof6_establish_session(&socket_fd, &proof_session_id)) {
        proof6_print("FAIL establish");
        goto done;
    }

    if (pstvnc_transport_session_open_with_mpeg(
            &socket_fd,
            &transport_config,
            &mpeg_config) != PSTVNC_TRANSPORT_OK) {
        proof6_print("FAIL transport_open");
        goto done;
    }
    transport_open = 1;

    if (pstvnc_transport_access_acquire(&control_access) !=
        PSTVNC_TRANSPORT_OK) {
        proof6_print("FAIL control_access");
        goto done;
    }

    if (!proof6_worker_start(&worker)) {
        proof6_print("FAIL worker_start");
        goto done;
    }

    for (loops = 0u; loops < PROOF6_STOP_WAIT_LOOPS; loops++) {
        if (worker.finished)
            break;

        if (!stop_requested &&
            worker.decoder_ready &&
            worker.platform.pictures_presented >=
                PROOF6_STOP_ARM_PICTURES &&
            worker.platform.in_mpeg_picture) {
            stop_requested_while_picture =
                worker.platform.in_mpeg_picture ? 1u : 0u;

            if (pstvnc_mpeg_decoder_request_stop(
                    &worker.decoder) !=
                PSTVNC_MPEG_DECODER_COMPLETE) {
                proof6_print("FAIL decoder_stop_request");
                goto done;
            }

            stop_requested = 1;
            break;
        }

        if (pstvnc_ps2_system_delay_us(PROOF6_POLL_DELAY_US) < 0) {
            proof6_print("FAIL stop_poll_delay");
            goto done;
        }
    }

    if (!stop_requested || !stop_requested_while_picture) {
        proof6_print("FAIL stop_not_requested_during_picture");
        goto done;
    }

    if (!proof6_worker_join(&worker)) {
        proof6_print("FAIL worker_join");
        goto done;
    }

    if (worker.initialize_result != PSTVNC_MPEG_DECODER_COMPLETE ||
        worker.run_result != PSTVNC_MPEG_DECODER_STOPPED ||
        worker.release_result != PSTVNC_MPEG_DECODER_COMPLETE ||
        worker.report.pictures_decoded < PROOF6_STOP_ARM_PICTURES ||
        worker.platform.pictures_presented < PROOF6_STOP_ARM_PICTURES) {
        proof6_print("FAIL decoder_lifecycle_result");
        goto done;
    }

    memcpy(stop_status, "MSTP", 4u);
    pstvnc_transport_write_be32(
        stop_status + 4u,
        worker.report.pictures_decoded);
    pstvnc_transport_write_be32(
        stop_status + 8u,
        worker.platform.pictures_presented);
    pstvnc_transport_write_be32(
        stop_status + 12u,
        worker.report.feed_callbacks);

    payload_bytes_low =
        worker.report.payload_bytes_consumed > UINT32_MAX
            ? UINT32_MAX
            : (uint32_t)worker.report.payload_bytes_consumed;

    pstvnc_transport_write_be32(
        stop_status + 16u,
        payload_bytes_low);

    if (pstvnc_transport_rfb_write_exact(
            &control_access,
            stop_status,
            sizeof(stop_status)) != PSTVNC_TRANSPORT_OK) {
        proof6_print("FAIL stop_status_send");
        goto done;
    }

    memset(ack, 0, sizeof(ack));
    if (!proof6_read_rfb_exact(&control_access, ack) ||
        memcmp(ack, "MRET", 4u) != 0) {
        proof6_print("FAIL producer_retire_ack");
        goto done;
    }

    if (pstvnc_transport_mpeg_mark_producer_done(
            &control_access) != PSTVNC_TRANSPORT_OK) {
        proof6_print("FAIL mark_producer_done");
        goto done;
    }

    if (!proof6_drain_mpeg_residual(
            &control_access,
            &residual_discarded)) {
        proof6_print("FAIL residual_drain");
        goto done;
    }

    memcpy(live_status, "LIVE", 4u);
    pstvnc_transport_write_be32(
        live_status + 4u,
        residual_discarded);
    pstvnc_transport_write_be32(
        live_status + 8u,
        stop_requested_while_picture);

    if (pstvnc_transport_rfb_write_exact(
            &control_access,
            live_status,
            sizeof(live_status)) != PSTVNC_TRANSPORT_OK) {
        proof6_print("FAIL live_status_send");
        goto done;
    }

    memset(ack, 0, sizeof(ack));
    if (!proof6_read_rfb_exact(&control_access, ack) ||
        memcmp(ack, "ROK!", 4u) != 0) {
        proof6_print("FAIL post_mpeg_rfb_roundtrip");
        goto done;
    }

    if (pstvnc_transport_rfb_write_exact(
            &control_access,
            "DONE",
            4u) != PSTVNC_TRANSPORT_OK) {
        proof6_print("FAIL done_marker");
        goto done;
    }

    if (!proof6_retire_closed_session()) {
        proof6_print("FAIL session_retire");
        goto done;
    }
    transport_open = 0;

    pass = 1;

done:
    if (!pass && transport_open)
        (void)pstvnc_transport_session_abort();

    if (worker.thread_started)
        (void)proof6_worker_join(&worker);

    if (worker.platform.graphics_ready) {
        proof6_mpeg_platform_show_solid(
            &worker.platform,
            pass ? 0 : 128,
            pass ? 96 : 0,
            0);
    }

    proof6_mpeg_platform_context_release(&worker.platform);

    if (worker.sync.semaphore_id >= 0) {
        (void)DeleteSema(worker.sync.semaphore_id);
        worker.sync.semaphore_id = -1;
    }

    if (socket_fd >= 0)
        pstvnc_ps2_network_close(socket_fd);

    if (pass) {
        char line[128];

        snprintf(
            line,
            sizeof(line),
            "PROOF6 PASS session=%u pics=%u residual=%u",
            (unsigned int)proof_session_id,
            (unsigned int)worker.report.pictures_decoded,
            (unsigned int)residual_discarded);
        printf("%s\n", line);
    } else {
        printf("PROOF6 FAIL\n");
    }

    SleepThread();
    return pass ? 0 : 1;
}

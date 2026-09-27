/*
 * File synopsis:
 * Standalone PS2 hardware discriminator for the R45 sole-owner wake question.
 * It is not PS-to-VNC product code. One EE owner thread alone performs socket
 * select/send, while one domain submitter publishes work through a semaphore
 * rendezvous immediately after the owner's first outbound-ready miss.
 *
 * Build variants:
 *   R45_ZERO_TIMEOUT_CONTROL=0 -> baseline 1000-us readiness, pre-R44 shape.
 *   R45_ZERO_TIMEOUT_CONTROL=1 -> zero-timeout plus second-poll R44 control.
 *
 * The peer must never send application data. PASS is represented only by the
 * owner successfully serializing every expected outbound record.
 */

#include <arpa/inet.h>
#include <delaythread.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <kernel.h>
#include <loadfile.h>
#include <netinet/in.h>
#include <ps2ip.h>
#include <sbv_patches.h>
#include <sifrpc.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "platform/ps2_network.h"

#ifndef R45_ZERO_TIMEOUT_CONTROL
#define R45_ZERO_TIMEOUT_CONTROL 0
#endif

#if R45_ZERO_TIMEOUT_CONTROL != 0 && R45_ZERO_TIMEOUT_CONTROL != 1
#error R45_ZERO_TIMEOUT_CONTROL must be 0 or 1
#endif

#define R45_PEER_IP "192.168.50.1"
#define R45_PEER_PORT 5961
#define R45_CYCLE_COUNT 4096u
#define R45_IDLE_YIELD_US 1000
#define R45_RECORD_SIZE 24u
#define R45_RECORD_VERSION 1u
#define R45_RECORD_KIND_COMPLETION 1u
#define R45_RECORD_MAGIC UINT32_C(0x52343557)

#if R45_ZERO_TIMEOUT_CONTROL
#define R45_VARIANT_CODE 2u
#define R45_VARIANT_NAME "CONTROL_ZERO_TIMEOUT"
#define R45_READINESS_TIMEOUT_US 0u
#else
#define R45_VARIANT_CODE 1u
#define R45_VARIANT_NAME "BASELINE_1000US"
#define R45_READINESS_TIMEOUT_US 1000u
#endif

static const char r45_variant_identity[] __attribute__((used)) =
    "R45_WAKE_DISCRIMINATOR_V1:" R45_VARIANT_NAME;

static unsigned char owner_stack[16 * 1024] __attribute__((aligned(16)));
static unsigned char submitter_stack[16 * 1024] __attribute__((aligned(16)));

typedef struct r45_context {
    int socket_fd;
    int outbound_ready_semaphore;
    int outbound_done_semaphore;
    int owner_missed_semaphore;
    int owner_complete_semaphore;
    int submitter_complete_semaphore;
    volatile uint32_t pending_cycle;
    volatile uint32_t pending_sequence;
    volatile uint32_t serialized_count;
    volatile uint32_t completed_cycle;
    volatile uint32_t failed_cycle;
    volatile int apparatus_invalid;
    volatile int owner_done;
    volatile int submitter_done;
} r45_context_t;

static void write_be32(uint8_t *out, uint32_t value)
{
    out[0] = (uint8_t)(value >> 24);
    out[1] = (uint8_t)(value >> 16);
    out[2] = (uint8_t)(value >> 8);
    out[3] = (uint8_t)value;
}

static uint32_t record_checksum(uint32_t cycle, uint32_t sequence)
{
    return R45_RECORD_MAGIC ^
        UINT32_C(0x01000000) ^
        ((uint32_t)R45_VARIANT_CODE << 16) ^
        ((uint32_t)R45_RECORD_KIND_COMPLETION << 8) ^
        cycle ^
        sequence ^
        (uint32_t)R45_READINESS_TIMEOUT_US;
}

static int send_exact(int socket_fd, const uint8_t *buffer, size_t count)
{
    size_t offset = 0u;

    while (offset < count) {
        int sent = send(
            socket_fd,
            buffer + offset,
            count - offset,
            0);

        if (sent <= 0)
            return 0;

        offset += (size_t)sent;
    }

    return 1;
}

static int send_completion_record(
    r45_context_t *context,
    uint32_t cycle,
    uint32_t sequence)
{
    uint8_t record[R45_RECORD_SIZE];

    memset(record, 0, sizeof(record));
    write_be32(&record[0], R45_RECORD_MAGIC);
    record[4] = (uint8_t)R45_RECORD_VERSION;
    record[5] = (uint8_t)R45_VARIANT_CODE;
    record[6] = (uint8_t)R45_RECORD_KIND_COMPLETION;
    record[7] = 0u;
    write_be32(&record[8], cycle);
    write_be32(&record[12], sequence);
    write_be32(&record[16], (uint32_t)R45_READINESS_TIMEOUT_US);
    write_be32(&record[20], record_checksum(cycle, sequence));

    return send_exact(context->socket_fd, record, sizeof(record));
}

static int wait_readable(int socket_fd, uint32_t timeout_us)
{
    fd_set read_set;
    struct timeval timeout;
    int result;

    FD_ZERO(&read_set);
    FD_SET(socket_fd, &read_set);

    timeout.tv_sec = (long)(timeout_us / UINT32_C(1000000));
    timeout.tv_usec = (long)(timeout_us % UINT32_C(1000000));

    result = select(
        socket_fd + 1,
        &read_set,
        NULL,
        NULL,
        &timeout);

    if (result < 0)
        return -1;
    if (result == 0)
        return 0;

    return FD_ISSET(socket_fd, &read_set) ? 1 : -1;
}

static int take_outbound_ready(r45_context_t *context)
{
    return PollSema(context->outbound_ready_semaphore) >= 0;
}

static int process_outbound(r45_context_t *context, uint32_t cycle)
{
    uint32_t pending_cycle = context->pending_cycle;
    uint32_t pending_sequence = context->pending_sequence;

    if (pending_cycle != cycle ||
        pending_sequence != cycle ||
        !send_completion_record(context, pending_cycle, pending_sequence)) {
        context->failed_cycle = cycle;
        context->apparatus_invalid = 1;
        return 0;
    }

    context->serialized_count++;
    context->completed_cycle = cycle;

    if (SignalSema(context->outbound_done_semaphore) < 0) {
        context->failed_cycle = cycle;
        context->apparatus_invalid = 1;
        return 0;
    }

    return 1;
}

static void owner_thread(void *opaque)
{
    r45_context_t *context = (r45_context_t *)opaque;
    uint32_t cycle;

    for (cycle = 1u; cycle <= R45_CYCLE_COUNT; cycle++) {
        int announced = 0;

        for (;;) {
            int readable;

            if (take_outbound_ready(context)) {
                if (!process_outbound(context, cycle))
                    goto done;
                break;
            }

            if (!announced) {
                /*
                 * This is the disputed interleaving boundary: PollSema has
                 * already missed. The submitter is released immediately before
                 * the owner enters the readiness phase.
                 */
                if (SignalSema(context->owner_missed_semaphore) < 0) {
                    context->failed_cycle = cycle;
                    context->apparatus_invalid = 1;
                    goto done;
                }
                announced = 1;
            }

            readable = wait_readable(
                context->socket_fd,
                R45_READINESS_TIMEOUT_US);

            if (readable != 0) {
                /*
                 * The recorder is contractually silent. Readability therefore
                 * means peer/application traffic, close, or socket failure and
                 * invalidates the apparatus rather than becoming a wake source.
                 */
                context->failed_cycle = cycle;
                context->apparatus_invalid = 1;
                goto done;
            }

#if R45_ZERO_TIMEOUT_CONTROL
            /*
             * R44 control shape: after the nonblocking readiness probe, close
             * the check/probe race with a second outbound-ready poll.
             */
            if (take_outbound_ready(context)) {
                if (!process_outbound(context, cycle))
                    goto done;
                break;
            }
#endif

            /*
             * Match the product's existing cooperative idle cadence. This is
             * never completion authority; only serialized send + done signal is.
             */
            if (DelayThread(R45_IDLE_YIELD_US) < 0) {
                context->failed_cycle = cycle;
                context->apparatus_invalid = 1;
                goto done;
            }
        }
    }

done:
    context->owner_done = 1;
    (void)SignalSema(context->owner_complete_semaphore);
}

static void submitter_thread(void *opaque)
{
    r45_context_t *context = (r45_context_t *)opaque;
    uint32_t cycle;

    for (cycle = 1u; cycle <= R45_CYCLE_COUNT; cycle++) {
        if (WaitSema(context->owner_missed_semaphore) < 0) {
            context->failed_cycle = cycle;
            context->apparatus_invalid = 1;
            break;
        }

        context->pending_cycle = cycle;
        context->pending_sequence = cycle;

        if (SignalSema(context->outbound_ready_semaphore) < 0) {
            context->failed_cycle = cycle;
            context->apparatus_invalid = 1;
            break;
        }

        /*
         * Actual owner serialization is the only completion fact. No delay,
         * retry count, or wall-clock threshold substitutes for this semaphore.
         */
        if (WaitSema(context->outbound_done_semaphore) < 0) {
            context->failed_cycle = cycle;
            context->apparatus_invalid = 1;
            break;
        }

        if (context->completed_cycle != cycle ||
            context->serialized_count != cycle) {
            context->failed_cycle = cycle;
            context->apparatus_invalid = 1;
            break;
        }
    }

    context->submitter_done = 1;
    (void)SignalSema(context->submitter_complete_semaphore);
}

static int create_semaphore(int initial_count, int max_count)
{
    ee_sema_t semaphore;

    memset(&semaphore, 0, sizeof(semaphore));
    semaphore.init_count = initial_count;
    semaphore.max_count = max_count;
    semaphore.option = 0;
    return CreateSema(&semaphore);
}

static int prepare_iop(void)
{
    sceSifInitRpc(0);

    while (!SifIopReset("", 0)) {
    }

    while (!SifIopSync()) {
    }

    sceSifInitRpc(0);

    if (SifLoadFileInit() < 0)
        return -1;
    if (SifInitIopHeap() < 0)
        return -1;
    if (sbv_patch_enable_lmb() < 0)
        return -1;

    return 0;
}

static int connect_recorder(void)
{
    struct sockaddr_in server;
    int socket_fd;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
        return -1;

    memset(&server, 0, sizeof(server));
    server.sin_len = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_port = htons(R45_PEER_PORT);
    server.sin_addr.s_addr = inet_addr(R45_PEER_IP);

    if (connect(
            socket_fd,
            (struct sockaddr *)&server,
            sizeof(server)) < 0) {
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}

static int start_thread(
    ee_thread_t *thread,
    void (*entry)(void *),
    void *stack,
    int stack_size,
    int priority,
    void *argument)
{
    int thread_id;

    memset(thread, 0, sizeof(*thread));
    thread->func = (void *)entry;
    thread->stack = stack;
    thread->stack_size = stack_size;
    thread->gp_reg = &_gp;
    thread->initial_priority = priority;
    thread->attr = 0;
    thread->option = 0;

    thread_id = CreateThread(thread);
    if (thread_id < 0)
        return -1;

    if (StartThread(thread_id, argument) < 0) {
        (void)DeleteThread(thread_id);
        return -1;
    }

    return thread_id;
}

int main(int argc, char **argv)
{
    r45_context_t context;
    ee_thread_t owner_definition;
    ee_thread_t submitter_definition;
    int owner_thread_id;
    int submitter_thread_id;

    (void)argc;
    (void)argv;
    (void)r45_variant_identity;

    memset(&context, 0, sizeof(context));
    context.socket_fd = -1;

    printf(
        "R45_VARIANT=%s TIMEOUT_US=%u CYCLES=%u\n",
        R45_VARIANT_NAME,
        (unsigned int)R45_READINESS_TIMEOUT_US,
        (unsigned int)R45_CYCLE_COUNT);

    if (prepare_iop() < 0 ||
        pstvnc_ps2_network_init() < 0 ||
        pstvnc_ps2_network_wait_link() < 0)
        goto failed;

    context.socket_fd = connect_recorder();
    if (context.socket_fd < 0)
        goto failed;

    context.outbound_ready_semaphore = create_semaphore(0, 1);
    context.outbound_done_semaphore = create_semaphore(0, 1);
    context.owner_missed_semaphore = create_semaphore(0, 1);
    context.owner_complete_semaphore = create_semaphore(0, 1);
    context.submitter_complete_semaphore = create_semaphore(0, 1);

    if (context.outbound_ready_semaphore < 0 ||
        context.outbound_done_semaphore < 0 ||
        context.owner_missed_semaphore < 0 ||
        context.owner_complete_semaphore < 0 ||
        context.submitter_complete_semaphore < 0)
        goto failed;

    owner_thread_id = start_thread(
        &owner_definition,
        owner_thread,
        owner_stack,
        (int)sizeof(owner_stack),
        63,
        &context);
    if (owner_thread_id < 0)
        goto failed;

    submitter_thread_id = start_thread(
        &submitter_definition,
        submitter_thread,
        submitter_stack,
        (int)sizeof(submitter_stack),
        64,
        &context);
    if (submitter_thread_id < 0)
        goto failed;

    if (WaitSema(context.owner_complete_semaphore) < 0 ||
        WaitSema(context.submitter_complete_semaphore) < 0)
        goto failed;

    if (!context.apparatus_invalid &&
        context.serialized_count == R45_CYCLE_COUNT &&
        context.completed_cycle == R45_CYCLE_COUNT &&
        context.owner_done &&
        context.submitter_done) {
        printf(
            "R45_LOCAL_RESULT=PASS VARIANT=%s SERIALIZED=%u\n",
            R45_VARIANT_NAME,
            (unsigned int)context.serialized_count);
        close(context.socket_fd);
        SleepThread();
        return 0;
    }

failed:
    printf(
        "R45_LOCAL_RESULT=APPARATUS_INVALID VARIANT=%s FAILED_CYCLE=%u SERIALIZED=%u\n",
        R45_VARIANT_NAME,
        (unsigned int)context.failed_cycle,
        (unsigned int)context.serialized_count);
    if (context.socket_fd >= 0)
        close(context.socket_fd);
    SleepThread();
    return 1;
}

/*
 * File synopsis:
 * Hardware proof for the Q12 Transport in-flight admission/drain lifetime
 * fence.
 *
 * One low-priority EE proof worker enters the real product Transport bridge
 * under Session A and requests four RFB bytes.  The Pi supplies only one byte.
 * Its observation of the corresponding one-byte returned credit proves that
 * the worker crossed Transport admission, consumed valid A data, and remains
 * inside that same Transport read waiting for three more bytes.
 *
 * The Pi then kills A.  Controlled EE priorities make the main proof thread
 * run after receiver terminality but before the blocked worker can resume.
 * Session close must therefore return WOULD_BLOCK while the A call is still
 * admitted, and a replacement session-open probe must remain rejected.
 *
 * Only after the old Transport call returns may A runtime storage be released
 * and real Session B be adopted.  B emits DRAINOK! only after every preceding
 * assertion passed.
 *
 * This proof changes no product Transport behavior.  Q4 Wire establishment,
 * worker orchestration, scheduling, and the replacement probe are proof-local
 * apparatus.
 */

#include <debug.h>
#include <kernel.h>

#include <sys/socket.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "transport/bridge.h"
#include "transport/protocol.h"
#include "transport/transport.h"

#include "wire_establish_protocol.h"

#define Q12_HW2_MAIN_PRIORITY         40
#define Q12_HW2_WORKER_PRIORITY       80
#define Q12_HW2_WORKER_STACK_BYTES 16384

static const uint8_t q12_hw2_a_marker[4] = {
    'A', 'O', 'K', '!'
};

static const uint8_t q12_hw2_b_marker[8] = {
    'D', 'R', 'A', 'I', 'N', 'O', 'K', '!'
};

static pstvnc_transport_access_t q12_hw2_access_a;

static volatile int q12_hw2_worker_started;
static volatile int q12_hw2_worker_returned;
static volatile int q12_hw2_worker_result;

static int q12_hw2_worker_done_semaphore = -1;
static uint8_t q12_hw2_worker_buffer[4];

static uint8_t q12_hw2_worker_stack[Q12_HW2_WORKER_STACK_BYTES]
    __attribute__((aligned(16)));

static void q12_hw2_print(const char *message)
{
    printf("%s\n", message);
    scr_printf("%s\n", message);
}

static int q12_hw2_send_exact(
    int socket_fd,
    const void *buffer,
    size_t count)
{
    const uint8_t *bytes = (const uint8_t *)buffer;
    size_t sent = 0u;

    while (sent < count) {
        int result = send(
            socket_fd,
            bytes + sent,
            count - sent,
            0);

        if (result <= 0)
            return 0;

        sent += (size_t)result;
    }

    return 1;
}

static int q12_hw2_receive_exact(
    int socket_fd,
    void *buffer,
    size_t count)
{
    uint8_t *bytes = (uint8_t *)buffer;
    size_t received = 0u;

    while (received < count) {
        int result = recv(
            socket_fd,
            bytes + received,
            count - received,
            0);

        if (result <= 0)
            return 0;

        received += (size_t)result;
    }

    return 1;
}

static int q12_hw2_establish_session(
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

    if (!pstvnc_wire_proof_hello_encode(
            hello_payload,
            &hello))
        goto fail;

    memset(&header, 0, sizeof(header));
    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = PSTVNC_TRANSPORT_FRAME_HELLO;
    header.channel = PSTVNC_TRANSPORT_CHANNEL_CONTROL;
    header.sequence = 1u;
    header.payload_length = sizeof(hello_payload);

    if (!pstvnc_transport_header_encode(
            wire_header,
            &header) ||
        !q12_hw2_send_exact(
            *socket_fd,
            wire_header,
            sizeof(wire_header)) ||
        !q12_hw2_send_exact(
            *socket_fd,
            hello_payload,
            sizeof(hello_payload)))
        goto fail;

    if (!q12_hw2_receive_exact(
            *socket_fd,
            wire_header,
            sizeof(wire_header)) ||
        !pstvnc_transport_header_decode(
            &header,
            wire_header) ||
        header.kind != PSTVNC_WIRE_PROOF_FRAME_ACCEPT ||
        header.channel != PSTVNC_TRANSPORT_CHANNEL_CONTROL ||
        header.flags != 0u ||
        header.sequence != 1u ||
        header.payload_length != sizeof(result_payload) ||
        !q12_hw2_receive_exact(
            *socket_fd,
            result_payload,
            sizeof(result_payload)) ||
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

static pstvnc_transport_session_config_t q12_hw2_make_config(void)
{
    pstvnc_transport_session_config_t config;

    memset(&config, 0, sizeof(config));
    config.rfb_queue_capacity = 16u;
    config.rfb_initial_credit_bytes = 16u;
    config.rfb_credit_batch_bytes = 1u;
    config.rfb_credit_flush_on_empty = 1;
    config.rfb_credit_return_enabled = 1;
    config.receiver_thread_stack_size = 16384u;
    config.receiver_thread_priority = 63;
    config.max_data_payload = 16u;
    return config;
}

static int q12_hw2_create_semaphore(
    int initial_count,
    int maximum_count)
{
    ee_sema_t semaphore;

    memset(&semaphore, 0, sizeof(semaphore));
    semaphore.init_count = initial_count;
    semaphore.max_count = maximum_count;
    semaphore.option = 0;

    return CreateSema(&semaphore);
}

static void q12_hw2_worker(void *argument)
{
    (void)argument;

    q12_hw2_worker_started = 1;

    q12_hw2_worker_result =
        (int)pstvnc_transport_rfb_read_exact(
            &q12_hw2_access_a,
            q12_hw2_worker_buffer,
            sizeof(q12_hw2_worker_buffer));

    /*
     * This flag is published only after the public Transport call has
     * returned, which means Transport's admission count must already have
     * dropped.  The proof intentionally does not wait for this EE thread to
     * terminate before allowing Transport reuse.
     */
    q12_hw2_worker_returned = 1;

    if (q12_hw2_worker_done_semaphore >= 0)
        (void)SignalSema(q12_hw2_worker_done_semaphore);

    ExitThread();
}

static int q12_hw2_start_worker(int *worker_thread_id)
{
    ee_thread_t thread;

    if (worker_thread_id == NULL)
        return 0;

    memset(&thread, 0, sizeof(thread));
    thread.func = (void *)q12_hw2_worker;
    thread.stack = q12_hw2_worker_stack;
    thread.stack_size = sizeof(q12_hw2_worker_stack);
    thread.gp_reg = &_gp;
    thread.initial_priority = Q12_HW2_WORKER_PRIORITY;
    thread.attr = 0;
    thread.option = 0;

    *worker_thread_id = CreateThread(&thread);
    if (*worker_thread_id < 0)
        return 0;

    if (StartThread(*worker_thread_id, NULL) < 0) {
        (void)DeleteThread(*worker_thread_id);
        *worker_thread_id = -1;
        return 0;
    }

    return 1;
}

static int q12_hw2_worker_result_is_terminal(void)
{
    return q12_hw2_worker_result == PSTVNC_TRANSPORT_CLOSED ||
        q12_hw2_worker_result == PSTVNC_TRANSPORT_FAILED;
}

int main(int argc, char **argv)
{
    pstvnc_transport_session_config_t config =
        q12_hw2_make_config();

    pstvnc_transport_access_t access_b;
    pstvnc_transport_result_t result;
    ee_thread_status_t main_status;
    ee_thread_status_t worker_status;

    uint32_t session_a = 0u;
    uint32_t session_b = 0u;

    int socket_a = -1;
    int socket_b = -1;
    int replacement_probe_fd = -1;
    int replacement_probe_original = -1;

    int main_thread_id;
    int original_main_priority;
    int worker_thread_id = -1;

    char line[160];

    (void)argc;
    (void)argv;

    memset(&q12_hw2_access_a, 0, sizeof(q12_hw2_access_a));
    memset(&access_b, 0, sizeof(access_b));
    memset(q12_hw2_worker_buffer, 0, sizeof(q12_hw2_worker_buffer));

    q12_hw2_worker_started = 0;
    q12_hw2_worker_returned = 0;
    q12_hw2_worker_result = PSTVNC_TRANSPORT_INVALID;

    init_scr();

    q12_hw2_print("WIRE Q12 IN-FLIGHT DRAIN HARDWARE PROOF");
    q12_hw2_print("state=BOOT");
    q12_hw2_print("Q4_ESTABLISHMENT=PROOF_LOCAL");
    q12_hw2_print("Q12_TRANSPORT_BRIDGE=PRODUCT_SOURCE");

    if (pstvnc_ps2_system_prepare_iop() < 0) {
        q12_hw2_print("FAIL prepare_iop");
        SleepThread();
        return 10;
    }

    if (pstvnc_ps2_network_init() < 0 ||
        pstvnc_ps2_network_wait_link() < 0) {
        q12_hw2_print("FAIL network");
        SleepThread();
        return 11;
    }

    main_thread_id = GetThreadId();
    memset(&main_status, 0, sizeof(main_status));

    if (main_thread_id < 0 ||
        ReferThreadStatus(main_thread_id, &main_status) < 0) {
        q12_hw2_print("FAIL main_thread_status");
        SleepThread();
        return 12;
    }

    original_main_priority = main_status.current_priority;

    snprintf(
        line,
        sizeof(line),
        "MAIN_PRIORITY original=%d controlled=%d receiver=63 worker=%d",
        original_main_priority,
        Q12_HW2_MAIN_PRIORITY,
        Q12_HW2_WORKER_PRIORITY);
    q12_hw2_print(line);

    q12_hw2_print("state=SESSION_A_ESTABLISH");

    if (!q12_hw2_establish_session(
            &socket_a,
            &session_a)) {
        q12_hw2_print("FAIL session_a_establish");
        SleepThread();
        return 13;
    }

    if (pstvnc_transport_session_open(
            &socket_a,
            &config) != PSTVNC_TRANSPORT_OK) {
        q12_hw2_print("FAIL session_a_transport_open");
        SleepThread();
        return 14;
    }

    if (pstvnc_transport_access_acquire(
            &q12_hw2_access_a) != PSTVNC_TRANSPORT_OK) {
        q12_hw2_print("FAIL access_a_acquire");
        SleepThread();
        return 15;
    }

    snprintf(
        line,
        sizeof(line),
        "SESSION_A ACTIVE proof_id=%u",
        (unsigned int)session_a);
    q12_hw2_print(line);

    if (pstvnc_transport_rfb_write_exact(
            &q12_hw2_access_a,
            q12_hw2_a_marker,
            sizeof(q12_hw2_a_marker)) != PSTVNC_TRANSPORT_OK) {
        q12_hw2_print("FAIL access_a_valid_write");
        SleepThread();
        return 16;
    }

    q12_hw2_print("ACCESS_A_VALID_WRITE=PASS");

    q12_hw2_worker_done_semaphore =
        q12_hw2_create_semaphore(0, 1);

    if (q12_hw2_worker_done_semaphore < 0) {
        q12_hw2_print("FAIL worker_done_semaphore");
        SleepThread();
        return 17;
    }

    /*
     * Lower number is higher EE priority.  Once A terminality is published:
     *
     *     proof main = 40
     *     Transport I/O owner = 63
     *     admitted proof worker = 80
     *
     * The receiver therefore publishes terminality, wakes the worker, then
     * wakes main.  Main preempts before the worker can return through
     * access_finish(), making the close-WOULD_BLOCK observation deterministic.
     */
    if (ChangeThreadPriority(
            main_thread_id,
            Q12_HW2_MAIN_PRIORITY) < 0) {
        q12_hw2_print("FAIL main_priority_control");
        SleepThread();
        return 18;
    }

    if (!q12_hw2_start_worker(&worker_thread_id)) {
        q12_hw2_print("FAIL worker_start");
        SleepThread();
        return 19;
    }

    q12_hw2_print("state=A_WORKER_WAITING_FOR_4_BYTES");
    q12_hw2_print("EXPECT_PI_PARTIAL_BYTE_AND_RETURNED_CREDIT");

    /*
     * The Pi does not kill A until it has observed the one-byte returned
     * credit.  Therefore this wait can complete only after the proof server has
     * externally established that the worker entered the public Transport read
     * and consumed exactly one of its requested four bytes.
     */
    if (pstvnc_transport_session_wait_receiver_done() !=
        PSTVNC_TRANSPORT_OK) {
        q12_hw2_print("FAIL session_a_wait_receiver_done");
        SleepThread();
        return 20;
    }

    if (!q12_hw2_worker_started) {
        q12_hw2_print("FAIL worker_never_started");
        SleepThread();
        return 21;
    }

    if (q12_hw2_worker_returned) {
        q12_hw2_print("FAIL worker_returned_before_retirement_probe");
        SleepThread();
        return 22;
    }

    memset(&worker_status, 0, sizeof(worker_status));
    if (ReferThreadStatus(
            worker_thread_id,
            &worker_status) < 0) {
        q12_hw2_print("FAIL worker_status_after_a_loss");
        SleepThread();
        return 23;
    }

    snprintf(
        line,
        sizeof(line),
        "WORKER_AT_A_LOSS status=0x%x waitType=%u returned=%d",
        worker_status.status,
        (unsigned int)worker_status.waitType,
        (int)q12_hw2_worker_returned);
    q12_hw2_print(line);

    result = pstvnc_transport_session_close();

    if (result != PSTVNC_TRANSPORT_WOULD_BLOCK) {
        snprintf(
            line,
            sizeof(line),
            "FAIL first_close_result=%d",
            (int)result);
        q12_hw2_print(line);
        SleepThread();
        return 24;
    }

    if (q12_hw2_worker_returned) {
        q12_hw2_print("FAIL worker_returned_before_first_close_observation");
        SleepThread();
        return 25;
    }

    q12_hw2_print("IN_FLIGHT_A_BLOCKS_RUNTIME_RELEASE=PASS");

    /*
     * Use a valid but deliberately unconnected socket solely to exercise the
     * replacement-open gate.  A correctly fenced retiring singleton rejects
     * the attempt before it can adopt or mutate this descriptor.
     */
    replacement_probe_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (replacement_probe_fd < 0) {
        q12_hw2_print("FAIL replacement_probe_socket");
        SleepThread();
        return 26;
    }

    replacement_probe_original = replacement_probe_fd;

    result = pstvnc_transport_session_open(
        &replacement_probe_fd,
        &config);

    if (result != PSTVNC_TRANSPORT_INVALID ||
        replacement_probe_fd != replacement_probe_original) {
        snprintf(
            line,
            sizeof(line),
            "FAIL replacement_probe result=%d fd_before=%d fd_after=%d",
            (int)result,
            replacement_probe_original,
            replacement_probe_fd);
        q12_hw2_print(line);
        SleepThread();
        return 27;
    }

    if (q12_hw2_worker_returned) {
        q12_hw2_print("FAIL worker_returned_before_replacement_probe");
        SleepThread();
        return 28;
    }

    q12_hw2_print("B_OPEN_WHILE_A_CALL_ADMITTED=BLOCKED");

    /*
     * Main now intentionally yields.  The worker may finish its terminal A
     * read.  Its done semaphore is signalled only after the public Transport
     * call has returned, so an ensuing successful close proves reclamation was
     * delayed exactly through the Transport call and no farther.
     */
    if (WaitSema(q12_hw2_worker_done_semaphore) < 0) {
        q12_hw2_print("FAIL worker_done_wait");
        SleepThread();
        return 29;
    }

    if (!q12_hw2_worker_returned ||
        !q12_hw2_worker_result_is_terminal()) {
        snprintf(
            line,
            sizeof(line),
            "FAIL worker_terminal_result=%d returned=%d",
            (int)q12_hw2_worker_result,
            (int)q12_hw2_worker_returned);
        q12_hw2_print(line);
        SleepThread();
        return 30;
    }

    snprintf(
        line,
        sizeof(line),
        "A_CALL_RETURNED_TERMINAL result=%d",
        (int)q12_hw2_worker_result);
    q12_hw2_print(line);

    pstvnc_ps2_network_close(replacement_probe_fd);
    replacement_probe_fd = -1;

    result = pstvnc_transport_session_close();

    if (result != PSTVNC_TRANSPORT_OK) {
        snprintf(
            line,
            sizeof(line),
            "FAIL second_close_result=%d",
            (int)result);
        q12_hw2_print(line);
        SleepThread();
        return 31;
    }

    q12_hw2_print("A_RELEASE_AFTER_CALL_RETURN=PASS");

    if (ChangeThreadPriority(
            main_thread_id,
            original_main_priority) < 0) {
        q12_hw2_print("FAIL restore_main_priority");
        SleepThread();
        return 32;
    }

    q12_hw2_print("state=SESSION_B_ESTABLISH");

    if (!q12_hw2_establish_session(
            &socket_b,
            &session_b)) {
        q12_hw2_print("FAIL session_b_establish");
        SleepThread();
        return 33;
    }

    if (session_b == session_a) {
        q12_hw2_print("FAIL proof_session_id_reused");
        pstvnc_ps2_network_close(socket_b);
        SleepThread();
        return 34;
    }

    if (pstvnc_transport_session_open(
            &socket_b,
            &config) != PSTVNC_TRANSPORT_OK) {
        q12_hw2_print("FAIL session_b_transport_open");
        SleepThread();
        return 35;
    }

    if (pstvnc_transport_access_acquire(
            &access_b) != PSTVNC_TRANSPORT_OK) {
        q12_hw2_print("FAIL access_b_acquire");
        SleepThread();
        return 36;
    }

    snprintf(
        line,
        sizeof(line),
        "SESSION_B ACTIVE proof_id=%u",
        (unsigned int)session_b);
    q12_hw2_print(line);

    /*
     * The Pi treats this payload as the machine-readable assertion that every
     * preceding PS2-side drain/replacement check passed.
     */
    if (pstvnc_transport_rfb_write_exact(
            &access_b,
            q12_hw2_b_marker,
            sizeof(q12_hw2_b_marker)) != PSTVNC_TRANSPORT_OK) {
        q12_hw2_print("FAIL b_drain_marker");
        SleepThread();
        return 37;
    }

    q12_hw2_print("DRAIN_MARKER_SENT=PASS");
    q12_hw2_print("state=WAIT_SESSION_B_CLOSE");

    if (pstvnc_transport_session_wait_receiver_done() !=
        PSTVNC_TRANSPORT_OK) {
        q12_hw2_print("FAIL session_b_wait_receiver_done");
        SleepThread();
        return 38;
    }

    if (pstvnc_transport_session_close() !=
        PSTVNC_TRANSPORT_OK) {
        q12_hw2_print("FAIL session_b_close");
        SleepThread();
        return 39;
    }

    q12_hw2_print("SESSION_B_RETIRED=YES");
    q12_hw2_print("IN_FLIGHT_A_OPERATION_DRAIN_HARDWARE_PROOF=PASS");
    q12_hw2_print("FULL_Q12_ARCHITECTURAL_FENCE_CANDIDATE=PASS");
    q12_hw2_print("Q12 HW2 COMPLETE");

    SleepThread();
    return 0;
}

/*
 * File synopsis:
 * Direct host behavior fixture for src/transport/physical_stream.c. The real
 * physical-stream and protocol implementations are linked against deterministic
 * socket/semaphore primitives so framing, exact I/O, sequence progression,
 * serialized-send ownership, fatal-I/O interruption, and release ownership are
 * tested without replacing the behavior under test.
 *
 * Context: LEDGE_FOREMAN_STATE revision 0003, packet C1/C5.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "transport/physical_stream.h"
#include "transport/protocol.h"

#define TEST_SOCKET_FD 17
#define TEST_BUFFER_CAPACITY 512u

static int failures;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

typedef struct fake_socket_state {
    uint8_t inbound[TEST_BUFFER_CAPACITY];
    size_t inbound_length;
    size_t inbound_offset;
    size_t recv_chunk;
    int recv_calls;
    int recv_fail_on_call;

    uint8_t outbound[TEST_BUFFER_CAPACITY];
    size_t outbound_length;
    size_t send_chunk;
    int send_calls;
    int send_fail_on_call;
    int send_without_lock;

    int shutdown_calls;
    int shutdown_how;
    int shutdown_result;
    int close_calls;
} fake_socket_state_t;

static fake_socket_state_t fake_socket;
static int create_sema_calls;
static int create_sema_fail;
static int delete_sema_calls;
static int wait_sema_calls;
static int signal_sema_calls;
static int wait_sema_fail;
static int signal_sema_fail;
static int fake_sema_count;
static int fake_send_lock_depth;

unsigned char _gp;

static void reset_fixture(void)
{
    memset(&fake_socket, 0, sizeof(fake_socket));
    fake_socket.recv_chunk = TEST_BUFFER_CAPACITY;
    fake_socket.send_chunk = TEST_BUFFER_CAPACITY;
    fake_socket.shutdown_result = 0;

    create_sema_calls = 0;
    create_sema_fail = 0;
    delete_sema_calls = 0;
    wait_sema_calls = 0;
    signal_sema_calls = 0;
    wait_sema_fail = 0;
    signal_sema_fail = 0;
    fake_sema_count = 0;
    fake_send_lock_depth = 0;
}

int CreateSema(ee_sema_t *semaphore)
{
    create_sema_calls++;
    if (create_sema_fail || semaphore == NULL)
        return -1;

    fake_sema_count = semaphore->init_count;
    return 3;
}

int DeleteSema(int semaphore_id)
{
    CHECK(semaphore_id == 3);
    delete_sema_calls++;
    fake_sema_count = 0;
    return 0;
}

int WaitSema(int semaphore_id)
{
    CHECK(semaphore_id == 3);
    wait_sema_calls++;
    if (wait_sema_fail || fake_sema_count <= 0)
        return -1;

    fake_sema_count--;
    fake_send_lock_depth++;
    return 0;
}

int SignalSema(int semaphore_id)
{
    CHECK(semaphore_id == 3);
    signal_sema_calls++;
    if (signal_sema_fail)
        return -1;

    CHECK(fake_send_lock_depth == 1);
    if (fake_send_lock_depth > 0)
        fake_send_lock_depth--;
    fake_sema_count++;
    return 0;
}

ssize_t send(int socket_fd, const void *buffer, size_t length, int flags)
{
    size_t chunk = length;

    (void)flags;
    CHECK(socket_fd == TEST_SOCKET_FD);
    fake_socket.send_calls++;

    if (fake_send_lock_depth != 1)
        fake_socket.send_without_lock = 1;

    if (fake_socket.send_fail_on_call > 0 &&
        fake_socket.send_calls == fake_socket.send_fail_on_call)
        return 0;

    if (fake_socket.send_chunk < chunk)
        chunk = fake_socket.send_chunk;
    CHECK(fake_socket.outbound_length + chunk <= TEST_BUFFER_CAPACITY);
    if (fake_socket.outbound_length + chunk > TEST_BUFFER_CAPACITY)
        return -1;

    memcpy(
        fake_socket.outbound + fake_socket.outbound_length,
        buffer,
        chunk);
    fake_socket.outbound_length += chunk;
    return (ssize_t)chunk;
}

ssize_t recv(int socket_fd, void *buffer, size_t length, int flags)
{
    size_t available;
    size_t chunk = length;

    (void)flags;
    CHECK(socket_fd == TEST_SOCKET_FD);
    fake_socket.recv_calls++;

    if (fake_socket.recv_fail_on_call > 0 &&
        fake_socket.recv_calls == fake_socket.recv_fail_on_call)
        return 0;

    available = fake_socket.inbound_length - fake_socket.inbound_offset;
    if (available == 0u)
        return 0;
    if (chunk > available)
        chunk = available;
    if (fake_socket.recv_chunk < chunk)
        chunk = fake_socket.recv_chunk;

    memcpy(buffer, fake_socket.inbound + fake_socket.inbound_offset, chunk);
    fake_socket.inbound_offset += chunk;
    return (ssize_t)chunk;
}

int shutdown(int socket_fd, int how)
{
    CHECK(socket_fd == TEST_SOCKET_FD);
    fake_socket.shutdown_calls++;
    fake_socket.shutdown_how = how;
    return fake_socket.shutdown_result;
}

int close(int socket_fd)
{
    CHECK(socket_fd == TEST_SOCKET_FD);
    fake_socket.close_calls++;
    return 0;
}

static void append_inbound_frame(
    uint8_t kind,
    uint8_t channel,
    uint8_t flags,
    uint32_t sequence,
    const uint8_t *payload,
    size_t payload_length)
{
    pstvnc_transport_header_t header;
    uint8_t wire_header[PSTVNC_TRANSPORT_HEADER_SIZE];

    memset(&header, 0, sizeof(header));
    header.version = PSTVNC_TRANSPORT_VERSION;
    header.kind = kind;
    header.channel = channel;
    header.flags = flags;
    header.sequence = sequence;
    header.payload_length = (uint32_t)payload_length;

    CHECK(pstvnc_transport_header_encode(wire_header, &header) == 1);
    CHECK(fake_socket.inbound_length + sizeof(wire_header) + payload_length <=
        TEST_BUFFER_CAPACITY);

    memcpy(
        fake_socket.inbound + fake_socket.inbound_length,
        wire_header,
        sizeof(wire_header));
    fake_socket.inbound_length += sizeof(wire_header);

    if (payload_length != 0u) {
        memcpy(
            fake_socket.inbound + fake_socket.inbound_length,
            payload,
            payload_length);
        fake_socket.inbound_length += payload_length;
    }
}

static pstvnc_transport_physical_stream_t adopt_stream(void)
{
    pstvnc_transport_physical_stream_t stream;

    memset(&stream, 0xa5, sizeof(stream));
    CHECK(pstvnc_transport_physical_stream_adopt(
        &stream, TEST_SOCKET_FD) == 1);
    CHECK(stream.socket_fd == TEST_SOCKET_FD);
    CHECK(stream.send_semaphore_id == 3);
    CHECK(stream.next_send_sequence == 1u);
    CHECK(stream.expected_receive_sequence == 1u);
    return stream;
}

static void test_adopt_failure_keeps_socket_unowned(void)
{
    pstvnc_transport_physical_stream_t stream;

    reset_fixture();
    memset(&stream, 0, sizeof(stream));
    create_sema_fail = 1;

    CHECK(pstvnc_transport_physical_stream_adopt(
        &stream, TEST_SOCKET_FD) == 0);
    CHECK(stream.socket_fd == -1);
    CHECK(stream.send_semaphore_id == -1);
    CHECK(fake_socket.close_calls == 0);
}

static void test_complete_send_serializes_and_advances_sequence(void)
{
    static const uint8_t payload[] = { 0x10, 0x20, 0x30, 0x40, 0x50 };
    pstvnc_transport_physical_stream_t stream;
    pstvnc_transport_header_t header;

    reset_fixture();
    fake_socket.send_chunk = 3u;
    stream = adopt_stream();

    CHECK(pstvnc_transport_physical_stream_send_frame(
        &stream,
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        payload,
        sizeof(payload)) == 1);

    CHECK(fake_socket.send_without_lock == 0);
    CHECK(wait_sema_calls == 1);
    CHECK(signal_sema_calls == 1);
    CHECK(stream.next_send_sequence == 2u);
    CHECK(fake_socket.outbound_length ==
        PSTVNC_TRANSPORT_HEADER_SIZE + sizeof(payload));
    CHECK(pstvnc_transport_header_decode(
        &header, fake_socket.outbound) == 1);
    CHECK(header.kind == PSTVNC_TRANSPORT_FRAME_DATA);
    CHECK(header.channel == PSTVNC_TRANSPORT_CHANNEL_RFB);
    CHECK(header.sequence == 1u);
    CHECK(header.payload_length == sizeof(payload));
    CHECK(memcmp(
        fake_socket.outbound + PSTVNC_TRANSPORT_HEADER_SIZE,
        payload,
        sizeof(payload)) == 0);

    pstvnc_transport_physical_stream_release(&stream);
}

static void test_failed_send_does_not_advance_sequence(void)
{
    static const uint8_t payload[] = { 1, 2, 3, 4 };
    pstvnc_transport_physical_stream_t stream;

    reset_fixture();
    stream = adopt_stream();
    fake_socket.send_fail_on_call = 2; /* header succeeds, payload fails */

    CHECK(pstvnc_transport_physical_stream_send_frame(
        &stream,
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        payload,
        sizeof(payload)) == 0);
    CHECK(stream.next_send_sequence == 1u);
    CHECK(wait_sema_calls == 1);
    CHECK(signal_sema_calls == 1);

    pstvnc_transport_physical_stream_release(&stream);
}

static void test_send_lock_failure_never_touches_socket(void)
{
    pstvnc_transport_physical_stream_t stream;

    reset_fixture();
    stream = adopt_stream();
    wait_sema_fail = 1;

    CHECK(pstvnc_transport_physical_stream_send_frame(
        &stream,
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        NULL,
        0u) == 0);
    CHECK(fake_socket.send_calls == 0);
    CHECK(stream.next_send_sequence == 1u);

    pstvnc_transport_physical_stream_release(&stream);
}

static void test_complete_receive_advances_only_after_payload(void)
{
    static const uint8_t payload[] = { 7, 8, 9, 10, 11 };
    uint8_t output[sizeof(payload)];
    pstvnc_transport_physical_stream_t stream;
    pstvnc_transport_header_t header;

    reset_fixture();
    fake_socket.recv_chunk = 2u;
    append_inbound_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        1u,
        payload,
        sizeof(payload));
    stream = adopt_stream();

    memset(output, 0, sizeof(output));
    CHECK(pstvnc_transport_physical_stream_receive_frame(
        &stream, &header, output, sizeof(output)) == 1);
    CHECK(stream.expected_receive_sequence == 2u);
    CHECK(header.sequence == 1u);
    CHECK(header.payload_length == sizeof(payload));
    CHECK(memcmp(output, payload, sizeof(payload)) == 0);

    pstvnc_transport_physical_stream_release(&stream);
}

static void test_receive_rejects_wrong_sequence_without_progress(void)
{
    pstvnc_transport_physical_stream_t stream;
    pstvnc_transport_header_t header;

    reset_fixture();
    append_inbound_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        2u,
        NULL,
        0u);
    stream = adopt_stream();

    CHECK(pstvnc_transport_physical_stream_receive_frame(
        &stream, &header, NULL, 0u) == 0);
    CHECK(stream.expected_receive_sequence == 1u);

    pstvnc_transport_physical_stream_release(&stream);
}

static void test_receive_rejects_invalid_protocol_without_progress(void)
{
    pstvnc_transport_physical_stream_t stream;
    pstvnc_transport_header_t header;

    reset_fixture();
    append_inbound_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        1u,
        NULL,
        0u);
    fake_socket.inbound[0] ^= 0xffu; /* corrupt PSTV magic */
    stream = adopt_stream();

    CHECK(pstvnc_transport_physical_stream_receive_frame(
        &stream, &header, NULL, 0u) == 0);
    CHECK(stream.expected_receive_sequence == 1u);

    pstvnc_transport_physical_stream_release(&stream);
}

static void test_short_receive_payload_does_not_advance_sequence(void)
{
    static const uint8_t payload[] = { 1, 2, 3 };
    uint8_t output[sizeof(payload)];
    pstvnc_transport_physical_stream_t stream;
    pstvnc_transport_header_t header;

    reset_fixture();
    append_inbound_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        1u,
        payload,
        sizeof(payload));
    fake_socket.inbound_length--; /* make the final payload byte unavailable */
    stream = adopt_stream();

    CHECK(pstvnc_transport_physical_stream_receive_frame(
        &stream, &header, output, sizeof(output)) == 0);
    CHECK(stream.expected_receive_sequence == 1u);

    pstvnc_transport_physical_stream_release(&stream);
}

static void test_receive_capacity_rejection_does_not_advance_sequence(void)
{
    static const uint8_t payload[] = { 1, 2, 3, 4 };
    uint8_t output[2];
    pstvnc_transport_physical_stream_t stream;
    pstvnc_transport_header_t header;

    reset_fixture();
    append_inbound_frame(
        PSTVNC_TRANSPORT_FRAME_DATA,
        PSTVNC_TRANSPORT_CHANNEL_RFB,
        0u,
        1u,
        payload,
        sizeof(payload));
    stream = adopt_stream();

    CHECK(pstvnc_transport_physical_stream_receive_frame(
        &stream, &header, output, sizeof(output)) == 0);
    CHECK(stream.expected_receive_sequence == 1u);
    CHECK(fake_socket.inbound_offset == PSTVNC_TRANSPORT_HEADER_SIZE);

    pstvnc_transport_physical_stream_release(&stream);
}

static void test_shutdown_interrupt_preserves_descriptor_ownership(void)
{
    pstvnc_transport_physical_stream_t stream;

    reset_fixture();
    stream = adopt_stream();

    CHECK(pstvnc_transport_physical_stream_shutdown_io(&stream) == 1);
    CHECK(fake_socket.shutdown_calls == 1);
    CHECK(fake_socket.shutdown_how == SHUT_RDWR);
    CHECK(fake_socket.close_calls == 0);
    CHECK(stream.socket_fd == TEST_SOCKET_FD);

    pstvnc_transport_physical_stream_release(&stream);
    CHECK(fake_socket.close_calls == 1);
    CHECK(delete_sema_calls == 1);
    CHECK(stream.socket_fd == -1);
    CHECK(stream.send_semaphore_id == -1);
    CHECK(stream.next_send_sequence == 1u);
    CHECK(stream.expected_receive_sequence == 1u);
}

int main(void)
{
    test_adopt_failure_keeps_socket_unowned();
    test_complete_send_serializes_and_advances_sequence();
    test_failed_send_does_not_advance_sequence();
    test_send_lock_failure_never_touches_socket();
    test_complete_receive_advances_only_after_payload();
    test_receive_rejects_wrong_sequence_without_progress();
    test_receive_rejects_invalid_protocol_without_progress();
    test_short_receive_payload_does_not_advance_sequence();
    test_receive_capacity_rejection_does_not_advance_sequence();
    test_shutdown_interrupt_preserves_descriptor_ownership();

    if (failures != 0) {
        fprintf(stderr,
            "transport_physical_stream_test: %d failure(s)\n",
            failures);
        return 1;
    }

    puts("transport_physical_stream_test: PASS");
    return 0;
}

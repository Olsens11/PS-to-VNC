#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "management/config_get.h"

#define ARRAY_COUNT(values) (sizeof(values) / sizeof((values)[0]))
#define TEST_SEND_PLAN_MAX 32u
#define TEST_RECV_PLAN_MAX 64u
#define TEST_SENT_BYTES_MAX 512u

static int failures;
static int connect_result;
static int connect_calls;
static int close_calls;
static int last_closed_fd;
static const int transport_sentinel_fd = 77;
static int send_plan[TEST_SEND_PLAN_MAX];
static size_t send_plan_count;
static size_t send_plan_index;
static unsigned char sent_bytes[TEST_SENT_BYTES_MAX];
static size_t sent_length;
static const unsigned char *response_bytes;
static size_t response_length;
static size_t response_offset;
static int recv_plan[TEST_RECV_PLAN_MAX];
static size_t recv_plan_count;
static size_t recv_plan_index;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "CHECK failed line %d: %s\n", __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static void reset_io(const unsigned char *response, size_t length)
{
    connect_result = 41;
    connect_calls = 0;
    close_calls = 0;
    last_closed_fd = -1;
    memset(send_plan, 0, sizeof(send_plan));
    send_plan_count = 0u;
    send_plan_index = 0u;
    memset(sent_bytes, 0, sizeof(sent_bytes));
    sent_length = 0u;
    response_bytes = response;
    response_length = length;
    response_offset = 0u;
    memset(recv_plan, 0, sizeof(recv_plan));
    recv_plan_count = 0u;
    recv_plan_index = 0u;
}

static void reset_text_response(const char *response)
{
    reset_io((const unsigned char *)response, strlen(response));
}

static void add_send_result(int result)
{
    CHECK(send_plan_count < ARRAY_COUNT(send_plan));
    if (send_plan_count < ARRAY_COUNT(send_plan))
        send_plan[send_plan_count++] = result;
}

static void add_recv_result(int result)
{
    CHECK(recv_plan_count < ARRAY_COUNT(recv_plan));
    if (recv_plan_count < ARRAY_COUNT(recv_plan))
        recv_plan[recv_plan_count++] = result;
}

int pstvnc_ps2_network_connect_management(void)
{
    connect_calls++;
    return connect_result;
}

void pstvnc_ps2_network_close(int socket_fd)
{
    close_calls++;
    last_closed_fd = socket_fd;
}

ssize_t __wrap_send(
    int socket_fd,
    const void *buffer,
    size_t length,
    int flags)
{
    int result;

    (void)flags;
    CHECK(socket_fd == connect_result);

    if (send_plan_index < send_plan_count)
        result = send_plan[send_plan_index++];
    else
        result = (int)length;

    if (result <= 0)
        return result;

    if ((size_t)result > length)
        result = (int)length;

    CHECK(sent_length + (size_t)result <= sizeof(sent_bytes));
    if (sent_length + (size_t)result > sizeof(sent_bytes))
        return -1;

    memcpy(sent_bytes + sent_length, buffer, (size_t)result);
    sent_length += (size_t)result;
    return result;
}

ssize_t __wrap_recv(
    int socket_fd,
    void *buffer,
    size_t length,
    int flags)
{
    int result;
    size_t remaining;

    (void)flags;
    CHECK(socket_fd == connect_result);

    if (recv_plan_index < recv_plan_count)
        result = recv_plan[recv_plan_index++];
    else if (response_offset < response_length)
        result = (int)(response_length - response_offset);
    else
        result = 0;

    if (result <= 0)
        return result;

    remaining = response_length - response_offset;
    if ((size_t)result > length)
        result = (int)length;
    if ((size_t)result > remaining)
        result = (int)remaining;
    if (result == 0)
        return 0;

    memcpy(buffer, response_bytes + response_offset, (size_t)result);
    response_offset += (size_t)result;
    return result;
}

static void check_single_management_close(void)
{
    CHECK(connect_calls == 1);
    CHECK(close_calls == 1);
    CHECK(last_closed_fd == connect_result);
    CHECK(last_closed_fd != transport_sentinel_fd);
}

static void test_exact_request_partial_send_and_split_header(void)
{
    static const char expected_request[] =
        "GET /ps2vnc.conf HTTP/1.0\r\n"
        "Host: 192.168.50.1\r\n"
        "Connection: close\r\n"
        "\r\n";
    static const char response[] =
        "HTTP/1.0 200 OK\r\n"
        "X-Test: split\r\n"
        "\r\n"
        "abc";
    const char *terminator = strstr(response, "\r\n\r\n");
    size_t terminator_offset;
    unsigned char output[8];
    size_t output_length = 999u;

    CHECK(terminator != NULL);
    if (terminator == NULL)
        return;

    terminator_offset = (size_t)(terminator - response);
    reset_text_response(response);
    add_send_result(3);
    add_send_result(2);
    add_send_result(7);
    add_recv_result((int)(terminator_offset + 1u));
    add_recv_result(1);
    add_recv_result(1);
    add_recv_result(4);
    add_recv_result(0);

    memset(output, 0, sizeof(output));
    CHECK(pstvnc_management_config_get(output, 3u, &output_length) == 0);
    CHECK(output_length == 3u);
    CHECK(memcmp(output, "abc", 3u) == 0);
    CHECK(sent_length == sizeof(expected_request) - 1u);
    CHECK(memcmp(sent_bytes, expected_request, sent_length) == 0);
    check_single_management_close();
}

static void test_http11_coalesced_and_raw_body(void)
{
    static const unsigned char response[] = {
        'H','T','T','P','/','1','.','1',' ','2','0','0',' ','O','K','\r','\n',
        'X',':',' ','y','\r','\n','\r','\n',
        'A',0u,'B'
    };
    unsigned char output[8];
    size_t output_length = 444u;

    reset_io(response, sizeof(response));
    memset(output, 0xee, sizeof(output));

    CHECK(pstvnc_management_config_get(output, 3u, &output_length) == 0);
    CHECK(output_length == 3u);
    CHECK(output[0] == 'A');
    CHECK(output[1] == 0u);
    CHECK(output[2] == 'B');
    check_single_management_close();
}

static void test_empty_body_is_valid(void)
{
    size_t output_length = 123u;

    reset_text_response("HTTP/1.0 200\r\n\r\n");
    CHECK(pstvnc_management_config_get(NULL, 0u, &output_length) == 0);
    CHECK(output_length == 0u);
    check_single_management_close();
}

static void test_status_rejections_are_atomic(void)
{
    static const char * const responses[] = {
        "HTTP/1.0 404 Not Found\r\n\r\nbody",
        "HTTP/1.1 201 Created\r\n\r\nbody",
        "HTTP/1.2 200 OK\r\n\r\nbody",
        "HTTP/2.0 200 OK\r\n\r\nbody",
        "HTTP/1.0 200X\r\n\r\nbody",
        "HTTP/1.0 20\r\n\r\nbody",
        "OK\r\n\r\nbody"
    };
    size_t i;

    for (i = 0u; i < ARRAY_COUNT(responses); i++) {
        unsigned char output[8];
        unsigned char before[sizeof(output)];
        size_t output_length = 321u;

        reset_text_response(responses[i]);
        memset(output, 0xa5, sizeof(output));
        memcpy(before, output, sizeof(before));

        CHECK(pstvnc_management_config_get(
            output,
            sizeof(output),
            &output_length) < 0);
        CHECK(memcmp(output, before, sizeof(output)) == 0);
        CHECK(output_length == 321u);
        check_single_management_close();
    }
}

static void test_missing_and_oversized_header_reject(void)
{
    unsigned char oversized[PSTVNC_MANAGEMENT_HTTP_MAX_HEADER_BYTES + 1u];
    unsigned char output[8];
    unsigned char before[sizeof(output)];
    size_t output_length;

    reset_text_response("HTTP/1.0 200 OK\r\nX: y\r\n");
    memset(output, 0xb6, sizeof(output));
    memcpy(before, output, sizeof(before));
    output_length = 901u;
    CHECK(pstvnc_management_config_get(output, sizeof(output), &output_length) < 0);
    CHECK(memcmp(output, before, sizeof(output)) == 0);
    CHECK(output_length == 901u);
    check_single_management_close();

    memset(oversized, 'A', sizeof(oversized));
    memcpy(oversized, "HTTP/1.0 200 OK\r\n", 17u);
    reset_io(oversized, sizeof(oversized));
    memset(output, 0xc7, sizeof(output));
    memcpy(before, output, sizeof(before));
    output_length = 902u;
    CHECK(pstvnc_management_config_get(output, sizeof(output), &output_length) < 0);
    CHECK(memcmp(output, before, sizeof(output)) == 0);
    CHECK(output_length == 902u);
    check_single_management_close();
}

static void test_exact_capacity_and_one_extra_byte(void)
{
    unsigned char output[4];
    unsigned char before[sizeof(output)];
    size_t output_length;

    reset_text_response("HTTP/1.0 200 OK\r\n\r\nabcd");
    memset(output, 0, sizeof(output));
    output_length = 811u;
    CHECK(pstvnc_management_config_get(output, sizeof(output), &output_length) == 0);
    CHECK(output_length == sizeof(output));
    CHECK(memcmp(output, "abcd", sizeof(output)) == 0);
    check_single_management_close();

    reset_text_response("HTTP/1.0 200 OK\r\n\r\nabcde");
    memset(output, 0xd8, sizeof(output));
    memcpy(before, output, sizeof(before));
    output_length = 812u;
    CHECK(pstvnc_management_config_get(output, sizeof(output), &output_length) < 0);
    CHECK(memcmp(output, before, sizeof(output)) == 0);
    CHECK(output_length == 812u);
    check_single_management_close();
}

static void test_connect_send_and_receive_failures(void)
{
    unsigned char output[8];
    unsigned char before[sizeof(output)];
    size_t output_length;

    reset_text_response("HTTP/1.0 200 OK\r\n\r\nabc");
    connect_result = -1;
    memset(output, 0xe9, sizeof(output));
    memcpy(before, output, sizeof(before));
    output_length = 700u;
    CHECK(pstvnc_management_config_get(output, sizeof(output), &output_length) < 0);
    CHECK(connect_calls == 1);
    CHECK(close_calls == 0);
    CHECK(memcmp(output, before, sizeof(output)) == 0);
    CHECK(output_length == 700u);

    reset_text_response("HTTP/1.0 200 OK\r\n\r\nabc");
    add_send_result(5);
    add_send_result(0);
    memset(output, 0xea, sizeof(output));
    memcpy(before, output, sizeof(before));
    output_length = 701u;
    CHECK(pstvnc_management_config_get(output, sizeof(output), &output_length) < 0);
    CHECK(memcmp(output, before, sizeof(output)) == 0);
    CHECK(output_length == 701u);
    check_single_management_close();

    reset_text_response("HTTP/1.0 200 OK\r\n\r\nabc");
    add_send_result(-1);
    memset(output, 0xeb, sizeof(output));
    memcpy(before, output, sizeof(before));
    output_length = 702u;
    CHECK(pstvnc_management_config_get(output, sizeof(output), &output_length) < 0);
    CHECK(memcmp(output, before, sizeof(output)) == 0);
    CHECK(output_length == 702u);
    check_single_management_close();

    reset_text_response("HTTP/1.0 200 OK\r\n\r\nabc");
    add_recv_result(12);
    add_recv_result(-1);
    memset(output, 0xec, sizeof(output));
    memcpy(before, output, sizeof(before));
    output_length = 703u;
    CHECK(pstvnc_management_config_get(output, sizeof(output), &output_length) < 0);
    CHECK(memcmp(output, before, sizeof(output)) == 0);
    CHECK(output_length == 703u);
    check_single_management_close();
}

static void test_argument_bounds_do_not_acquire_socket(void)
{
    unsigned char output[1];
    size_t output_length = 55u;

    reset_text_response("");
    CHECK(pstvnc_management_config_get(
        output,
        PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES + 1u,
        &output_length) < 0);
    CHECK(connect_calls == 0);
    CHECK(close_calls == 0);
    CHECK(output_length == 55u);

    reset_text_response("");
    CHECK(pstvnc_management_config_get(NULL, 1u, &output_length) < 0);
    CHECK(connect_calls == 0);
    CHECK(close_calls == 0);

    reset_text_response("");
    CHECK(pstvnc_management_config_get(output, 1u, NULL) < 0);
    CHECK(connect_calls == 0);
    CHECK(close_calls == 0);
}

int main(void)
{
    test_exact_request_partial_send_and_split_header();
    test_http11_coalesced_and_raw_body();
    test_empty_body_is_valid();
    test_status_rejections_are_atomic();
    test_missing_and_oversized_header_reject();
    test_exact_capacity_and_one_extra_byte();
    test_connect_send_and_receive_failures();
    test_argument_bounds_do_not_acquire_socket();

    if (failures != 0) {
        fprintf(stderr, "MANAGEMENT_CONFIG_GET_TEST=FAIL count=%d\n", failures);
        return 1;
    }

    puts("MANAGEMENT_CONFIG_GET_TEST=PASS");
    return 0;
}

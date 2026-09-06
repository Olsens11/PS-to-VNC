#include "transport_queue.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void test_basic_write_read(void)
{
    uint8_t storage[8];
    uint8_t output[5];

    const uint8_t input[5] = {
        1, 2, 3, 4, 5
    };

    pstvnc_transport_queue_t queue;

    assert(
        pstvnc_transport_queue_init(
            &queue,
            storage,
            sizeof(storage)));

    assert(
        pstvnc_transport_queue_size(
            &queue) == 0);

    assert(
        pstvnc_transport_queue_free(
            &queue) == sizeof(storage));

    assert(
        pstvnc_transport_queue_write(
            &queue,
            input,
            sizeof(input)));

    assert(
        pstvnc_transport_queue_size(
            &queue) == sizeof(input));

    assert(
        pstvnc_transport_queue_high_water(
            &queue) == sizeof(input));

    memset(output, 0, sizeof(output));

    assert(
        pstvnc_transport_queue_read(
            &queue,
            output,
            sizeof(output)));

    assert(
        memcmp(
            input,
            output,
            sizeof(input)) == 0);

    assert(
        pstvnc_transport_queue_size(
            &queue) == 0);
}

static void test_wraparound(void)
{
    uint8_t storage[8];
    uint8_t discarded[4];
    uint8_t output[8];

    const uint8_t first[6] = {
        10, 11, 12, 13, 14, 15
    };

    const uint8_t second[6] = {
        20, 21, 22, 23, 24, 25
    };

    const uint8_t expected[8] = {
        14, 15,
        20, 21, 22, 23, 24, 25
    };

    pstvnc_transport_queue_t queue;

    assert(
        pstvnc_transport_queue_init(
            &queue,
            storage,
            sizeof(storage)));

    assert(
        pstvnc_transport_queue_write(
            &queue,
            first,
            sizeof(first)));

    assert(
        pstvnc_transport_queue_read(
            &queue,
            discarded,
            sizeof(discarded)));

    assert(
        pstvnc_transport_queue_write(
            &queue,
            second,
            sizeof(second)));

    assert(
        pstvnc_transport_queue_size(
            &queue) == sizeof(storage));

    assert(
        pstvnc_transport_queue_high_water(
            &queue) == sizeof(storage));

    memset(output, 0, sizeof(output));

    assert(
        pstvnc_transport_queue_read(
            &queue,
            output,
            sizeof(output)));

    assert(
        memcmp(
            output,
            expected,
            sizeof(expected)) == 0);
}

static void test_failed_operations_are_atomic(void)
{
    uint8_t storage[4];

    const uint8_t initial[4] = {
        1, 2, 3, 4
    };

    const uint8_t extra = 5;

    uint8_t output[4];

    pstvnc_transport_queue_t queue;

    assert(
        pstvnc_transport_queue_init(
            &queue,
            storage,
            sizeof(storage)));

    assert(
        pstvnc_transport_queue_write(
            &queue,
            initial,
            sizeof(initial)));

    assert(
        !pstvnc_transport_queue_write(
            &queue,
            &extra,
            1));

    assert(
        pstvnc_transport_queue_size(
            &queue) == sizeof(initial));

    assert(
        !pstvnc_transport_queue_read(
            &queue,
            output,
            sizeof(output) + 1u));

    assert(
        pstvnc_transport_queue_size(
            &queue) == sizeof(initial));

    assert(
        pstvnc_transport_queue_read(
            &queue,
            output,
            sizeof(output)));

    assert(
        memcmp(
            initial,
            output,
            sizeof(initial)) == 0);
}

static void test_zero_length_operations(void)
{
    uint8_t storage[4];

    pstvnc_transport_queue_t queue;

    assert(
        pstvnc_transport_queue_init(
            &queue,
            storage,
            sizeof(storage)));

    assert(
        pstvnc_transport_queue_write(
            &queue,
            NULL,
            0));

    assert(
        pstvnc_transport_queue_read(
            &queue,
            NULL,
            0));

    assert(
        pstvnc_transport_queue_size(
            &queue) == 0);
}

int main(void)
{
    test_basic_write_read();
    test_wraparound();
    test_failed_operations_are_atomic();
    test_zero_length_operations();

    puts("TRANSPORT_QUEUE_TEST=PASS");
    return 0;
}

#include <stdio.h>
#include <string.h>

#include "rfb.h"

static int failures = 0;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

static void check_bytes(
    const unsigned char *actual,
    const unsigned char *expected,
    size_t count)
{
    CHECK(memcmp(actual, expected, count) == 0);
}

static void test_protocol_version(void)
{
    static const unsigned char good[12] = "RFB 003.008\n";
    unsigned char bad[12];
    unsigned int major = 99;
    unsigned int minor = 99;

    CHECK(pstvnc_rfb_parse_protocol_version(good, &major, &minor));
    CHECK(major == 3);
    CHECK(minor == 8);

    memcpy(bad, good, sizeof(bad));
    bad[4] = 'x';
    CHECK(!pstvnc_rfb_parse_protocol_version(bad, &major, &minor));

    memcpy(bad, good, sizeof(bad));
    bad[11] = '\r';
    CHECK(!pstvnc_rfb_parse_protocol_version(bad, &major, &minor));

    CHECK(!pstvnc_rfb_parse_protocol_version(NULL, &major, &minor));
    CHECK(!pstvnc_rfb_parse_protocol_version(good, NULL, &minor));
    CHECK(!pstvnc_rfb_parse_protocol_version(good, &major, NULL));
}

static void test_client_version(void)
{
    static const unsigned char expected[12] = "RFB 003.008\n";
    unsigned char actual[12];

    memset(actual, 0xa5, sizeof(actual));
    pstvnc_rfb_build_client_version(actual);
    check_bytes(actual, expected, sizeof(expected));
}

static void test_security(void)
{
    static const unsigned char offered[] = { 2, 16, 1, 19 };
    static const unsigned char missing[] = { 2, 16, 19 };
    static const unsigned char success[4] = { 0, 0, 0, 0 };
    static const unsigned char failure[4] = { 0, 0, 0, 1 };
    unsigned char choice = 0xff;

    CHECK(pstvnc_rfb_choose_security_none(
        offered, sizeof(offered), &choice));
    CHECK(choice == 1);
    CHECK(!pstvnc_rfb_choose_security_none(
        missing, sizeof(missing), &choice));
    CHECK(!pstvnc_rfb_choose_security_none(NULL, 1, &choice));
    CHECK(!pstvnc_rfb_choose_security_none(offered, 0, &choice));
    CHECK(!pstvnc_rfb_choose_security_none(
        offered, sizeof(offered), NULL));

    CHECK(pstvnc_rfb_security_result_ok(success));
    CHECK(!pstvnc_rfb_security_result_ok(failure));
    CHECK(!pstvnc_rfb_security_result_ok(NULL));
    CHECK(pstvnc_rfb_client_init_shared() == 1);
}

static void test_server_init(void)
{
    static const unsigned char bytes[24] = {
        0x02, 0xc0, 0x01, 0xce,
        32, 24, 0, 1,
        0, 255, 0, 255, 0, 255,
        16, 8, 0,
        0, 0, 0,
        0, 0, 0, 7
    };
    unsigned char zero_width[24];
    pstvnc_rfb_server_init_t init;

    CHECK(pstvnc_rfb_parse_server_init(bytes, &init));
    CHECK(init.width == 704);
    CHECK(init.height == 462);
    CHECK(init.bits_per_pixel == 32);
    CHECK(init.depth == 24);
    CHECK(init.big_endian == 0);
    CHECK(init.true_color == 1);
    CHECK(init.red_max == 255);
    CHECK(init.green_max == 255);
    CHECK(init.blue_max == 255);
    CHECK(init.red_shift == 16);
    CHECK(init.green_shift == 8);
    CHECK(init.blue_shift == 0);
    CHECK(init.name_length == 7);

    memcpy(zero_width, bytes, sizeof(zero_width));
    zero_width[0] = 0;
    zero_width[1] = 0;
    CHECK(!pstvnc_rfb_parse_server_init(zero_width, &init));
    CHECK(!pstvnc_rfb_parse_server_init(NULL, &init));
    CHECK(!pstvnc_rfb_parse_server_init(bytes, NULL));
}

static void test_set_pixel_format(void)
{
    static const unsigned char expected[20] = {
        0, 0, 0, 0,
        16, 15, 0, 1,
        0, 31,
        0, 31,
        0, 31,
        0, 5, 10,
        0, 0, 0
    };
    unsigned char actual[20];

    pstvnc_rfb_build_set_pixel_format_gs555(actual);
    check_bytes(actual, expected, sizeof(expected));
}

static void test_set_raw_encoding(void)
{
    static const unsigned char expected[8] = {
        2, 0,
        0, 1,
        0, 0, 0, 0
    };
    unsigned char actual[8];

    pstvnc_rfb_build_set_encodings_raw(actual);
    check_bytes(actual, expected, sizeof(expected));
}

static void test_framebuffer_request(void)
{
    static const unsigned char full_expected[10] = {
        3, 0,
        0, 0,
        0, 0,
        0x02, 0xc0,
        0x01, 0xce
    };
    static const unsigned char incremental_expected[10] = {
        3, 1,
        0, 0,
        0, 0,
        0x02, 0xc0,
        0x01, 0xce
    };
    unsigned char actual[10];

    pstvnc_rfb_build_framebuffer_update_request(
        actual, 0, 0, 0, 704, 462);
    check_bytes(actual, full_expected, sizeof(full_expected));

    pstvnc_rfb_build_framebuffer_update_request(
        actual, 1, 0, 0, 704, 462);
    check_bytes(actual, incremental_expected,
        sizeof(incremental_expected));
}


static void test_pointer_event(void)
{
    static const unsigned char expected[6] = {
        5,
        0x24,
        0x01, 0x23,
        0x04, 0x56
    };
    unsigned char actual[PSTVNC_RFB_POINTER_EVENT_SIZE];

    /*
     * Keep the protocol vocabulary explicit. The right button is RFB button 3,
     * while wheel-left is RFB button 6.
     */
    CHECK(PSTVNC_RFB_POINTER_BUTTON_LEFT == 0x01u);
    CHECK(PSTVNC_RFB_POINTER_BUTTON_MIDDLE == 0x02u);
    CHECK(PSTVNC_RFB_POINTER_BUTTON_RIGHT == 0x04u);
    CHECK(PSTVNC_RFB_POINTER_WHEEL_UP == 0x08u);
    CHECK(PSTVNC_RFB_POINTER_WHEEL_DOWN == 0x10u);
    CHECK(PSTVNC_RFB_POINTER_WHEEL_LEFT == 0x20u);
    CHECK(PSTVNC_RFB_POINTER_WHEEL_RIGHT == 0x40u);

    memset(actual, 0xa5, sizeof(actual));

    pstvnc_rfb_build_pointer_event(
        actual,
        (uint8_t)(
            PSTVNC_RFB_POINTER_BUTTON_RIGHT |
            PSTVNC_RFB_POINTER_WHEEL_LEFT),
        0x0123u,
        0x0456u);

    check_bytes(
        actual,
        expected,
        sizeof(expected));
}

int main(void)
{
    test_protocol_version();
    test_client_version();
    test_security();
    test_server_init();
    test_set_pixel_format();
    test_set_raw_encoding();
    test_framebuffer_request();
    test_pointer_event();

    if (failures != 0) {
        fprintf(stderr, "rfb_wire_test: %d failure(s)\n", failures);
        return 1;
    }

    puts("rfb_wire_test: PASS");
    return 0;
}

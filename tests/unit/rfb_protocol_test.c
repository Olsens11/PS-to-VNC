#include <stdio.h>
#include <string.h>

#include "../../src/rfb_protocol.h"

static int failures = 0;

static void expect_true(int condition, const char *name)
{
    if (condition)
        return;

    fprintf(stderr, "FAIL: %s\n", name);
    failures++;
}

static void write_be16(unsigned char *out, unsigned int value)
{
    out[0] = (unsigned char)((value >> 8) & 0xffu);
    out[1] = (unsigned char)(value & 0xffu);
}

static void write_be32(unsigned char *out, unsigned int value)
{
    out[0] = (unsigned char)((value >> 24) & 0xffu);
    out[1] = (unsigned char)((value >> 16) & 0xffu);
    out[2] = (unsigned char)((value >> 8) & 0xffu);
    out[3] = (unsigned char)(value & 0xffu);
}

static void test_version(void)
{
    static const unsigned char version38[12] = "RFB 003.008\n";
    static const unsigned char version37[12] = "RFB 003.007\n";

    expect_true(
        pstvnc_rfb_version_is_38(version38),
        "accept RFB 3.8");

    expect_true(
        !pstvnc_rfb_version_is_38(version37),
        "reject non-3.8 version");
}

static void test_security(void)
{
    static const unsigned char with_none[] = { 2, 1, 19 };
    static const unsigned char without_none[] = { 2, 19 };

    expect_true(
        pstvnc_rfb_security_none_offered(
            with_none,
            sizeof(with_none)),
        "find SecurityType None");

    expect_true(
        !pstvnc_rfb_security_none_offered(
            without_none,
            sizeof(without_none)),
        "reject security list without None");
}

static void test_server_init(void)
{
    unsigned char wire[PSTVNC_RFB_SERVER_INIT_LENGTH];
    pstvnc_rfb_server_init_t parsed;

    memset(wire, 0, sizeof(wire));
    memset(&parsed, 0, sizeof(parsed));

    write_be16(&wire[0], 1280);
    write_be16(&wire[2], 720);
    write_be32(&wire[20], 17);

    wire[4] = 16;
    wire[5] = 15;

    expect_true(
        pstvnc_rfb_parse_server_init(
            wire,
            1280,
            720,
            &parsed) == 0,
        "accept desktop inside explicit envelope");

    expect_true(
        parsed.width == 1280 &&
        parsed.height == 720 &&
        parsed.name_length == 17 &&
        parsed.pixel_format[0] == 16 &&
        parsed.pixel_format[1] == 15,
        "publish parsed ServerInit fields");

    expect_true(
        pstvnc_rfb_parse_server_init(
            wire,
            720,
            480,
            &parsed) < 0,
        "reject desktop outside explicit envelope");

    write_be16(&wire[0], 0);

    expect_true(
        pstvnc_rfb_parse_server_init(
            wire,
            1280,
            720,
            &parsed) < 0,
        "reject zero-width desktop");
}

int main(void)
{
    test_version();
    test_security();
    test_server_init();

    if (failures != 0) {
        fprintf(stderr, "RFB_PROTOCOL_TEST=FAIL count=%d\n", failures);
        return 1;
    }

    puts("RFB_PROTOCOL_TEST=PASS");
    return 0;
}

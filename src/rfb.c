#include <string.h>

#include "rfb.h"

/*
 * This file is the pure wire-format layer: it translates fixed RFB byte
 * layouts but owns no socket, session state, framebuffer, or recovery policy.
 * Keeping it side-effect free makes protocol details host-testable.
 */

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint32_t read_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) |
           (uint32_t)bytes[3];
}

static void write_be16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)((value >> 8) & 0xffu);
    bytes[1] = (uint8_t)(value & 0xffu);
}

int pstvnc_rfb_parse_protocol_version(
    const uint8_t banner[PSTVNC_RFB_PROTOCOL_VERSION_SIZE],
    unsigned int *major,
    unsigned int *minor)
{
    unsigned int i;
    unsigned int parsed_major = 0;
    unsigned int parsed_minor = 0;

    if (banner == NULL || major == NULL || minor == NULL)
        return 0;

    if (memcmp(banner, "RFB ", 4) != 0 ||
        banner[7] != '.' ||
        banner[11] != '\n')
        return 0;

    for (i = 4; i <= 6; i++) {
        if (banner[i] < '0' || banner[i] > '9')
            return 0;
        parsed_major = parsed_major * 10u +
            (unsigned int)(banner[i] - '0');
    }

    for (i = 8; i <= 10; i++) {
        if (banner[i] < '0' || banner[i] > '9')
            return 0;
        parsed_minor = parsed_minor * 10u +
            (unsigned int)(banner[i] - '0');
    }

    *major = parsed_major;
    *minor = parsed_minor;
    return 1;
}

void pstvnc_rfb_build_client_version(
    uint8_t out[PSTVNC_RFB_PROTOCOL_VERSION_SIZE])
{
    static const uint8_t version[PSTVNC_RFB_PROTOCOL_VERSION_SIZE] = {
        'R', 'F', 'B', ' ',
        '0', '0', '3', '.',
        '0', '0', '8', '\n'
    };

    memcpy(out, version, sizeof(version));
}

int pstvnc_rfb_choose_security_none(
    const uint8_t *types,
    size_t count,
    uint8_t *choice)
{
    size_t i;

    if (types == NULL || choice == NULL || count == 0)
        return 0;

    for (i = 0; i < count; i++) {
        if (types[i] == PSTVNC_RFB_SECURITY_NONE) {
            *choice = PSTVNC_RFB_SECURITY_NONE;
            return 1;
        }
    }

    return 0;
}

int pstvnc_rfb_security_result_ok(const uint8_t result[4])
{
    if (result == NULL)
        return 0;

    return read_be32(result) == 0;
}

uint8_t pstvnc_rfb_client_init_shared(void)
{
    return 1;
}

int pstvnc_rfb_parse_server_init(
    const uint8_t bytes[PSTVNC_RFB_SERVER_INIT_SIZE],
    pstvnc_rfb_server_init_t *out)
{
    if (bytes == NULL || out == NULL)
        return 0;

    out->width = read_be16(&bytes[0]);
    out->height = read_be16(&bytes[2]);
    out->bits_per_pixel = bytes[4];
    out->depth = bytes[5];
    out->big_endian = bytes[6];
    out->true_color = bytes[7];
    out->red_max = read_be16(&bytes[8]);
    out->green_max = read_be16(&bytes[10]);
    out->blue_max = read_be16(&bytes[12]);
    out->red_shift = bytes[14];
    out->green_shift = bytes[15];
    out->blue_shift = bytes[16];
    out->name_length = read_be32(&bytes[20]);

    return out->width != 0 && out->height != 0;
}

void pstvnc_rfb_build_set_pixel_format_gs555(
    uint8_t out[PSTVNC_RFB_SET_PIXEL_FORMAT_SIZE])
{
    /*
     * Request little-endian 16-bpp/depth-15 true color with five bits per
     * channel. Shifts 0/5/10 produce B5:G5:R5, matching the PS2 CT16 color bits
     * and avoiding a per-pixel channel rearrangement during presentation.
     */
    static const uint8_t message[PSTVNC_RFB_SET_PIXEL_FORMAT_SIZE] = {
        0, 0, 0, 0,
        16, 15, 0, 1,
        0, 31,
        0, 31,
        0, 31,
        0, 5, 10,
        0, 0, 0
    };

    memcpy(out, message, sizeof(message));
}

void pstvnc_rfb_build_set_encodings_raw(
    uint8_t out[PSTVNC_RFB_SET_ENCODINGS_RAW_SIZE])
{
    /*
     * Raw is the only advertised encoding for the qualification baseline. This
     * keeps the first hardware result independent of Hextile and other
     * performance paths that will be restored as separate capabilities.
     */
    static const uint8_t message[PSTVNC_RFB_SET_ENCODINGS_RAW_SIZE] = {
        2, 0,
        0, 1,
        0, 0, 0, 0
    };

    memcpy(out, message, sizeof(message));
}

void pstvnc_rfb_build_framebuffer_update_request(
    uint8_t out[PSTVNC_RFB_FRAMEBUFFER_REQUEST_SIZE],
    int incremental,
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height)
{
    memset(out, 0, PSTVNC_RFB_FRAMEBUFFER_REQUEST_SIZE);

    out[0] = 3;
    out[1] = incremental ? 1u : 0u;
    write_be16(&out[2], x);
    write_be16(&out[4], y);
    write_be16(&out[6], width);
    write_be16(&out[8], height);
}

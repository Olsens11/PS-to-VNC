#include "rfb_protocol.h"

#include <string.h>

static unsigned int pstvnc_rfb_read_be16(
    const unsigned char *data)
{
    return
        ((unsigned int)data[0] << 8) |
        (unsigned int)data[1];
}

static unsigned int pstvnc_rfb_read_be32(
    const unsigned char *data)
{
    return
        ((unsigned int)data[0] << 24) |
        ((unsigned int)data[1] << 16) |
        ((unsigned int)data[2] << 8) |
        (unsigned int)data[3];
}

int pstvnc_rfb_version_is_38(
    const unsigned char version[PSTVNC_RFB_VERSION_LENGTH])
{
    static const unsigned char expected[PSTVNC_RFB_VERSION_LENGTH] =
        "RFB 003.008\n";

    if (version == NULL)
        return 0;

    return memcmp(version, expected, sizeof(expected)) == 0;
}

int pstvnc_rfb_security_none_offered(
    const unsigned char *types,
    size_t count)
{
    size_t index;

    if (types == NULL || count == 0)
        return 0;

    for (index = 0; index < count; index++) {
        if (types[index] == 1)
            return 1;
    }

    return 0;
}

int pstvnc_rfb_parse_server_init(
    const unsigned char server_init[PSTVNC_RFB_SERVER_INIT_LENGTH],
    unsigned int max_width,
    unsigned int max_height,
    pstvnc_rfb_server_init_t *result)
{
    unsigned int width;
    unsigned int height;

    if (server_init == NULL || result == NULL)
        return -1;

    if (max_width == 0 || max_height == 0)
        return -1;

    width = pstvnc_rfb_read_be16(&server_init[0]);
    height = pstvnc_rfb_read_be16(&server_init[2]);

    if (width == 0 || height == 0)
        return -1;

    if (width > max_width || height > max_height)
        return -1;

    result->width = width;
    result->height = height;
    result->name_length = pstvnc_rfb_read_be32(&server_init[20]);

    memcpy(
        result->pixel_format,
        &server_init[4],
        sizeof(result->pixel_format));

    return 0;
}

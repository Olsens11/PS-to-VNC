#ifndef PSTVNC_RFB_PROTOCOL_H
#define PSTVNC_RFB_PROTOCOL_H

#include <stddef.h>

#define PSTVNC_RFB_VERSION_LENGTH 12u
#define PSTVNC_RFB_SERVER_INIT_LENGTH 24u

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int name_length;
    unsigned char pixel_format[16];
} pstvnc_rfb_server_init_t;

/*
 * The first clean reconstruction deliberately targets the same RFB 3.8
 * contract that the exploratory implementation proved against TigerVNC.
 */
int pstvnc_rfb_version_is_38(
    const unsigned char version[PSTVNC_RFB_VERSION_LENGTH]);

int pstvnc_rfb_security_none_offered(
    const unsigned char *types,
    size_t count);

/*
 * Parse and validate the fixed 24-byte ServerInit prefix.
 *
 * Geometry acceptance belongs to the caller. This function rejects zero-sized
 * desktops and values outside the explicit envelope; it does not acquire any
 * display-policy authority from the server.
 */
int pstvnc_rfb_parse_server_init(
    const unsigned char server_init[PSTVNC_RFB_SERVER_INIT_LENGTH],
    unsigned int max_width,
    unsigned int max_height,
    pstvnc_rfb_server_init_t *result);

#endif

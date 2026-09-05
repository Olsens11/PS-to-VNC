/*
 * File synopsis:
 * Defines RFB constants, parsed values, and pure wire helpers; sequencing and
 * transport live elsewhere.
 *
 * Context: docs/reconstruction/ISSUE7_MINIMAL_CORE.md, "RFB connection and
 * wire contract"; docs/CLEAN_ARCHITECTURE.md, "RFB client/session".
 */

#ifndef PSTVNC_RFB_H
#define PSTVNC_RFB_H

#include <stddef.h>
#include <stdint.h>

#define PSTVNC_RFB_PROTOCOL_VERSION_SIZE 12u
#define PSTVNC_RFB_SERVER_INIT_SIZE 24u
#define PSTVNC_RFB_SET_PIXEL_FORMAT_SIZE 20u
#define PSTVNC_RFB_SET_ENCODINGS_RAW_SIZE 8u
#define PSTVNC_RFB_FRAMEBUFFER_REQUEST_SIZE 10u
#define PSTVNC_RFB_POINTER_EVENT_SIZE 6u

#define PSTVNC_RFB_SECURITY_NONE 1u
#define PSTVNC_RFB_ENCODING_RAW 0

/*
 * Native RFB PointerEvent button-mask bits.
 *
 * These are protocol vocabulary, deliberately distinct from the semantic mouse
 * button vocabulary in mouse.h. In particular, semantic right-click is not
 * assumed to have the same bit value as RFB button 3.
 *
 * Historical qualified PS-to-VNC behavior uses RFB buttons 4-7 as wheel
 * up/down/left/right respectively. A wheel notch is represented by sending the
 * wheel bit and then sending the ordinary held-button mask again at the same
 * coordinates.
 */
#define PSTVNC_RFB_POINTER_BUTTON_LEFT        0x01u
#define PSTVNC_RFB_POINTER_BUTTON_MIDDLE      0x02u
#define PSTVNC_RFB_POINTER_BUTTON_RIGHT       0x04u
#define PSTVNC_RFB_POINTER_WHEEL_UP           0x08u
#define PSTVNC_RFB_POINTER_WHEEL_DOWN         0x10u
#define PSTVNC_RFB_POINTER_WHEEL_LEFT         0x20u
#define PSTVNC_RFB_POINTER_WHEEL_RIGHT        0x40u

typedef struct pstvnc_rfb_server_init {
    uint16_t width;
    uint16_t height;
    uint8_t bits_per_pixel;
    uint8_t depth;
    uint8_t big_endian;
    uint8_t true_color;
    uint16_t red_max;
    uint16_t green_max;
    uint16_t blue_max;
    uint8_t red_shift;
    uint8_t green_shift;
    uint8_t blue_shift;
    uint32_t name_length;
} pstvnc_rfb_server_init_t;

int pstvnc_rfb_parse_protocol_version(
    const uint8_t banner[PSTVNC_RFB_PROTOCOL_VERSION_SIZE],
    unsigned int *major,
    unsigned int *minor);

void pstvnc_rfb_build_client_version(
    uint8_t out[PSTVNC_RFB_PROTOCOL_VERSION_SIZE]);

int pstvnc_rfb_choose_security_none(
    const uint8_t *types,
    size_t count,
    uint8_t *choice);

int pstvnc_rfb_security_result_ok(const uint8_t result[4]);

uint8_t pstvnc_rfb_client_init_shared(void);

int pstvnc_rfb_parse_server_init(
    const uint8_t bytes[PSTVNC_RFB_SERVER_INIT_SIZE],
    pstvnc_rfb_server_init_t *out);

void pstvnc_rfb_build_set_pixel_format_gs555(
    uint8_t out[PSTVNC_RFB_SET_PIXEL_FORMAT_SIZE]);

void pstvnc_rfb_build_set_encodings_raw(
    uint8_t out[PSTVNC_RFB_SET_ENCODINGS_RAW_SIZE]);

void pstvnc_rfb_build_framebuffer_update_request(
    uint8_t out[PSTVNC_RFB_FRAMEBUFFER_REQUEST_SIZE],
    int incremental,
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height);

/*
 * Build one exact six-byte RFB PointerEvent.
 *
 * button_mask is already native RFB protocol vocabulary. This pure wire helper
 * does not know controller mappings, semantic mouse bits, wheel policy, or
 * application publication state.
 */
void pstvnc_rfb_build_pointer_event(
    uint8_t out[PSTVNC_RFB_POINTER_EVENT_SIZE],
    uint8_t button_mask,
    uint16_t x,
    uint16_t y);

#endif

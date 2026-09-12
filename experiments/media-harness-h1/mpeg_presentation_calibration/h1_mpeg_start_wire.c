/*
 * File synopsis:
 * Encodes and decodes the fixed CP2P MPEG-start payload used for the
 * generation-scoped PS2->Pi start handoff.
 *
 * Geometry is validated structurally on both encode and decode. The base MPEG
 * rectangle remains exact and 16-pixel aligned for the current codec path; the
 * suppression footprint must contain that base rectangle. Canvas bounds remain
 * the responsibility of the PS2 calibration/start-handoff owner and the Pi's
 * active-desktop validation because this payload intentionally carries no
 * duplicate canvas dimensions.
 */
#include "h1_mpeg_start_wire.h"

#include "transport_protocol.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static int h1_mpeg_start_geometry_valid(
    uint32_t generation,
    int draw_x,
    int draw_y,
    int draw_width,
    int draw_height,
    const pstvnc_mpeg_cal_rect_t *suppression)
{
    uint64_t draw_right;
    uint64_t draw_bottom;
    uint64_t suppression_right;
    uint64_t suppression_bottom;

    if (generation == 0u || suppression == NULL ||
        draw_x < 0 || draw_y < 0 ||
        draw_width < 16 || draw_height < 16 ||
        (draw_width & 15) != 0 || (draw_height & 15) != 0 ||
        suppression->x < 0 || suppression->y < 0 ||
        suppression->width <= 0 || suppression->height <= 0)
        return 0;

    draw_right = (uint64_t)(unsigned int)draw_x +
        (uint64_t)(unsigned int)draw_width;
    draw_bottom = (uint64_t)(unsigned int)draw_y +
        (uint64_t)(unsigned int)draw_height;
    suppression_right = (uint64_t)(unsigned int)suppression->x +
        (uint64_t)(unsigned int)suppression->width;
    suppression_bottom = (uint64_t)(unsigned int)suppression->y +
        (uint64_t)(unsigned int)suppression->height;

    return suppression->x <= draw_x &&
        suppression->y <= draw_y &&
        suppression_right >= draw_right &&
        suppression_bottom >= draw_bottom;
}

static int h1_mpeg_start_decode_int(uint32_t raw, int *value)
{
    if (value == NULL || raw > (uint32_t)INT_MAX)
        return 0;

    *value = (int)raw;
    return 1;
}

int pstvnc_h1_mpeg_start_wire_encode(
    uint8_t output[PSTVNC_H1_MPEG_START_WIRE_BYTES],
    uint32_t session_id,
    const pstvnc_h1_mpeg_start_contract_t *contract)
{
    if (output == NULL || contract == NULL ||
        !h1_mpeg_start_geometry_valid(
            contract->generation,
            contract->draw_x,
            contract->draw_y,
            contract->draw_width,
            contract->draw_height,
            &contract->suppression_rect))
        return 0;

    pstvnc_transport_write_be32(&output[0], PSTVNC_H1_MPEG_START_WIRE_VERSION);
    pstvnc_transport_write_be32(&output[4], session_id);
    pstvnc_transport_write_be32(&output[8], contract->generation);
    pstvnc_transport_write_be32(&output[12], (uint32_t)contract->draw_x);
    pstvnc_transport_write_be32(&output[16], (uint32_t)contract->draw_y);
    pstvnc_transport_write_be32(&output[20], (uint32_t)contract->draw_width);
    pstvnc_transport_write_be32(&output[24], (uint32_t)contract->draw_height);
    pstvnc_transport_write_be32(
        &output[28], (uint32_t)contract->suppression_rect.x);
    pstvnc_transport_write_be32(
        &output[32], (uint32_t)contract->suppression_rect.y);
    pstvnc_transport_write_be32(
        &output[36], (uint32_t)contract->suppression_rect.width);
    pstvnc_transport_write_be32(
        &output[40], (uint32_t)contract->suppression_rect.height);
    return 1;
}

int pstvnc_h1_mpeg_start_wire_decode(
    pstvnc_h1_mpeg_start_wire_message_t *message,
    const uint8_t input[PSTVNC_H1_MPEG_START_WIRE_BYTES])
{
    pstvnc_h1_mpeg_start_wire_message_t decoded;

    if (message == NULL || input == NULL ||
        pstvnc_transport_read_be32(&input[0]) !=
            PSTVNC_H1_MPEG_START_WIRE_VERSION)
        return 0;

    memset(&decoded, 0, sizeof(decoded));
    decoded.session_id = pstvnc_transport_read_be32(&input[4]);
    decoded.generation = pstvnc_transport_read_be32(&input[8]);

    if (!h1_mpeg_start_decode_int(
            pstvnc_transport_read_be32(&input[12]), &decoded.draw_x) ||
        !h1_mpeg_start_decode_int(
            pstvnc_transport_read_be32(&input[16]), &decoded.draw_y) ||
        !h1_mpeg_start_decode_int(
            pstvnc_transport_read_be32(&input[20]), &decoded.draw_width) ||
        !h1_mpeg_start_decode_int(
            pstvnc_transport_read_be32(&input[24]), &decoded.draw_height) ||
        !h1_mpeg_start_decode_int(
            pstvnc_transport_read_be32(&input[28]),
            &decoded.suppression_rect.x) ||
        !h1_mpeg_start_decode_int(
            pstvnc_transport_read_be32(&input[32]),
            &decoded.suppression_rect.y) ||
        !h1_mpeg_start_decode_int(
            pstvnc_transport_read_be32(&input[36]),
            &decoded.suppression_rect.width) ||
        !h1_mpeg_start_decode_int(
            pstvnc_transport_read_be32(&input[40]),
            &decoded.suppression_rect.height) ||
        !h1_mpeg_start_geometry_valid(
            decoded.generation,
            decoded.draw_x,
            decoded.draw_y,
            decoded.draw_width,
            decoded.draw_height,
            &decoded.suppression_rect))
        return 0;

    *message = decoded;
    return 1;
}

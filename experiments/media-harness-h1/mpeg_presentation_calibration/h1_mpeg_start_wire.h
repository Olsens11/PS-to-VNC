/*
 * File synopsis:
 * Defines the CP2P PS2->Pi MPEG-start payload carried as one PSTV DATA frame on
 * logical MPEG2 channel 4 after an accepted calibration region is committed.
 *
 * This module owns only the 44-byte payload representation. It does not own the
 * PSTV socket, frame sequencing, calibration state, MPEG production, RFB
 * suppression, or presentation lifecycle. The in-memory presentation contract
 * remains authoritative on the PS2; this wire form carries only the facts the
 * Pi needs to arm exact-region capture and generation-scoped suppression.
 */
#ifndef PSTVNC_H1_MPEG_START_WIRE_H
#define PSTVNC_H1_MPEG_START_WIRE_H

#include "h1_mpeg_start_handoff.h"

#include <stdint.h>

#define PSTVNC_H1_MPEG_START_WIRE_VERSION 1u
#define PSTVNC_H1_MPEG_START_WIRE_WORDS 11u
#define PSTVNC_H1_MPEG_START_WIRE_BYTES \
    (PSTVNC_H1_MPEG_START_WIRE_WORDS * 4u)

typedef struct pstvnc_h1_mpeg_start_wire_message {
    uint32_t session_id;
    uint32_t generation;

    int draw_x;
    int draw_y;
    int draw_width;
    int draw_height;

    pstvnc_mpeg_cal_rect_t suppression_rect;
} pstvnc_h1_mpeg_start_wire_message_t;

/*
 * Wire layout, all words big-endian:
 *
 *   0  version
 *   1  session_id
 *   2  generation
 *   3  draw_x
 *   4  draw_y
 *   5  draw_width
 *   6  draw_height
 *   7  suppression_x
 *   8  suppression_y
 *   9  suppression_width
 *  10  suppression_height
 *
 * Inner matte remains PS2 presentation state and is intentionally not sent to
 * the Pi. The Pi captures the exact base draw rectangle and suppresses RFB over
 * the separately encoded outer footprint.
 */
int pstvnc_h1_mpeg_start_wire_encode(
    uint8_t output[PSTVNC_H1_MPEG_START_WIRE_BYTES],
    uint32_t session_id,
    const pstvnc_h1_mpeg_start_contract_t *contract);

int pstvnc_h1_mpeg_start_wire_decode(
    pstvnc_h1_mpeg_start_wire_message_t *message,
    const uint8_t input[PSTVNC_H1_MPEG_START_WIRE_BYTES]);

#endif /* PSTVNC_H1_MPEG_START_WIRE_H */

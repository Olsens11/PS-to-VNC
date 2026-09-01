/*
 * File synopsis:
 * Defines the retained debug-stage vocabulary and transport boundary shared by
 * diagnostic producers.
 */

#ifndef PSTVNC_DIAGNOSTICS_DEBUG_H
#define PSTVNC_DIAGNOSTICS_DEBUG_H

#include <stddef.h>

/*
 * Permanent diagnostics/debug ownership boundary.
 *
 * This module owns:
 *   - debug-stage identity/state;
 *   - debug-stage naming;
 *   - the UDP diagnostic socket and destination;
 *   - raw diagnostic datagram transport.
 *
 * It deliberately does not own the application-domain values carried by
 * diagnostic records.  RFB, UI, input, video, calibration, management, and
 * profiling code supply their own observations to diagnostic formatters.
 */

typedef enum pstvnc_debug_stage {
    PSTVNC_DEBUG_STAGE_STARTUP = 0,
    PSTVNC_DEBUG_STAGE_NET_READY = 1,
    PSTVNC_DEBUG_STAGE_GS_INIT = 2,
    PSTVNC_DEBUG_STAGE_INITIAL_TEXTURE = 3,
    PSTVNC_DEBUG_STAGE_CONTROLLER_START = 4,
    PSTVNC_DEBUG_STAGE_LOOP_BEGIN = 5,
    PSTVNC_DEBUG_STAGE_PRE_DRAW = 6,
    PSTVNC_DEBUG_STAGE_POST_DRAW = 7,
    PSTVNC_DEBUG_STAGE_PRE_FLIP = 8,
    PSTVNC_DEBUG_STAGE_POST_FLIP = 9,
    PSTVNC_DEBUG_STAGE_PRE_QUEUE = 10,
    PSTVNC_DEBUG_STAGE_POST_QUEUE = 11,
    PSTVNC_DEBUG_STAGE_PRE_REQUEST = 12,
    PSTVNC_DEBUG_STAGE_POST_REQUEST = 13,
    PSTVNC_DEBUG_STAGE_WAIT_RFB = 14,
    PSTVNC_DEBUG_STAGE_POST_RECEIVE = 15,
    PSTVNC_DEBUG_STAGE_PRE_TEXTURE = 16,
    PSTVNC_DEBUG_STAGE_POST_TEXTURE = 17,
    PSTVNC_DEBUG_STAGE_CONTROLLER_ERROR = 18,
    PSTVNC_DEBUG_STAGE_REQUEST_ERROR = 19,
    PSTVNC_DEBUG_STAGE_RECEIVE_ERROR = 20
} pstvnc_debug_stage_t;

int pstvnc_debug_init(void);
int pstvnc_debug_is_ready(void);

int pstvnc_debug_send(
    const void *data,
    size_t length);

void pstvnc_debug_set_stage(
    pstvnc_debug_stage_t stage);

pstvnc_debug_stage_t pstvnc_debug_get_stage(void);

const char *pstvnc_debug_stage_name(
    pstvnc_debug_stage_t stage);

#endif

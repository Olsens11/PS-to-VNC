/*
 * File synopsis:
 * Defines the experiment-local RFB request scheduler used by MPEG calibration
 * integration to preserve one outstanding framebuffer request at a time while
 * honoring HOLD / INCREMENTAL / FULL safe-boundary policy.
 *
 * This module owns no socket, parser, framebuffer, controller, or presentation
 * state. It only tracks whether a framebuffer request is outstanding and
 * translates the calibration RFB gate into a request decision at an already-
 * proven complete-server-message/IDLE boundary.
 */

#ifndef PSTVNC_H1_MPEG_CALIBRATION_RFB_SCHEDULE_H
#define PSTVNC_H1_MPEG_CALIBRATION_RFB_SCHEDULE_H

#include "h1_mpeg_calibration_rfb_gate.h"

typedef enum pstvnc_h1_mpeg_calibration_rfb_request {
    PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_HOLD = 0,
    PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_INCREMENTAL,
    PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_FULL
} pstvnc_h1_mpeg_calibration_rfb_request_t;

typedef struct pstvnc_h1_mpeg_calibration_rfb_schedule {
    unsigned request_outstanding : 1;
} pstvnc_h1_mpeg_calibration_rfb_schedule_t;

void pstvnc_h1_mpeg_calibration_rfb_schedule_init(
    pstvnc_h1_mpeg_calibration_rfb_schedule_t *schedule);

/*
 * Evaluate the next framebuffer request only at an RFB-safe boundary.
 *
 * If a request is already outstanding, HOLD is returned without consuming a
 * pending post-thaw full-refresh obligation. This prevents IDLE servicing from
 * manufacturing duplicate FramebufferUpdateRequests while a static desktop is
 * legitimately waiting for damage.
 *
 * With no request outstanding:
 * - frozen gate -> HOLD;
 * - thawed gate with one-shot refresh obligation -> FULL;
 * - ordinary thawed gate -> INCREMENTAL.
 */
pstvnc_h1_mpeg_calibration_rfb_request_t
pstvnc_h1_mpeg_calibration_rfb_schedule_next(
    pstvnc_h1_mpeg_calibration_rfb_schedule_t *schedule,
    pstvnc_h1_mpeg_calibration_rfb_gate_t *gate);

/* Record successful transmission of the decision returned by _next(). */
int pstvnc_h1_mpeg_calibration_rfb_schedule_request_sent(
    pstvnc_h1_mpeg_calibration_rfb_schedule_t *schedule,
    pstvnc_h1_mpeg_calibration_rfb_request_t request);

/*
 * Record completion of one server framebuffer-update message. An update that
 * arrived for a request issued before calibration entry still clears the
 * outstanding-request fact even when visual publication is suppressed.
 */
int pstvnc_h1_mpeg_calibration_rfb_schedule_update_complete(
    pstvnc_h1_mpeg_calibration_rfb_schedule_t *schedule);

int pstvnc_h1_mpeg_calibration_rfb_schedule_has_outstanding_request(
    const pstvnc_h1_mpeg_calibration_rfb_schedule_t *schedule);

#endif /* PSTVNC_H1_MPEG_CALIBRATION_RFB_SCHEDULE_H */

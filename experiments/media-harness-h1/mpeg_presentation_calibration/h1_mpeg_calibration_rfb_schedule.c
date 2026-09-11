/*
 * File synopsis:
 * Implements the experiment-local MPEG calibration RFB request scheduler.
 */

#include "h1_mpeg_calibration_rfb_schedule.h"

#include <stddef.h>

void pstvnc_h1_mpeg_calibration_rfb_schedule_init(
    pstvnc_h1_mpeg_calibration_rfb_schedule_t *schedule)
{
    if (schedule == NULL)
        return;

    schedule->request_outstanding = 0u;
}

pstvnc_h1_mpeg_calibration_rfb_request_t
pstvnc_h1_mpeg_calibration_rfb_schedule_next(
    pstvnc_h1_mpeg_calibration_rfb_schedule_t *schedule,
    pstvnc_h1_mpeg_calibration_rfb_gate_t *gate)
{
    if (schedule == NULL || gate == NULL)
        return PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_HOLD;

    if (schedule->request_outstanding)
        return PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_HOLD;

    if (!pstvnc_h1_mpeg_calibration_rfb_gate_allows_request(gate))
        return PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_HOLD;

    if (pstvnc_h1_mpeg_calibration_rfb_gate_take_full_refresh(gate))
        return PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_FULL;

    return PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_INCREMENTAL;
}

int pstvnc_h1_mpeg_calibration_rfb_schedule_request_sent(
    pstvnc_h1_mpeg_calibration_rfb_schedule_t *schedule,
    pstvnc_h1_mpeg_calibration_rfb_request_t request)
{
    if (schedule == NULL ||
        request == PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_HOLD ||
        schedule->request_outstanding)
        return 0;

    if (request != PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_INCREMENTAL &&
        request != PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_FULL)
        return 0;

    schedule->request_outstanding = 1u;
    return 1;
}

int pstvnc_h1_mpeg_calibration_rfb_schedule_update_complete(
    pstvnc_h1_mpeg_calibration_rfb_schedule_t *schedule)
{
    if (schedule == NULL || !schedule->request_outstanding)
        return 0;

    schedule->request_outstanding = 0u;
    return 1;
}

int pstvnc_h1_mpeg_calibration_rfb_schedule_has_outstanding_request(
    const pstvnc_h1_mpeg_calibration_rfb_schedule_t *schedule)
{
    return schedule != NULL && schedule->request_outstanding;
}

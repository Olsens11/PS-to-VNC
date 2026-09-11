/*
 * File synopsis:
 * Host contract tests for the calibration-safe RFB request scheduler.
 */

#include <assert.h>
#include <stdio.h>

#include "h1_mpeg_calibration_rfb_gate.h"
#include "h1_mpeg_calibration_rfb_schedule.h"

static void test_ordinary_incremental_request(void)
{
    pstvnc_h1_mpeg_calibration_rfb_gate_t gate;
    pstvnc_h1_mpeg_calibration_rfb_schedule_t schedule;
    pstvnc_h1_mpeg_calibration_rfb_request_t request;

    pstvnc_h1_mpeg_calibration_rfb_gate_init(&gate);
    pstvnc_h1_mpeg_calibration_rfb_schedule_init(&schedule);

    request = pstvnc_h1_mpeg_calibration_rfb_schedule_next(
        &schedule, &gate);
    assert(request == PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_INCREMENTAL);
    assert(pstvnc_h1_mpeg_calibration_rfb_schedule_request_sent(
        &schedule, request));
    assert(pstvnc_h1_mpeg_calibration_rfb_schedule_has_outstanding_request(
        &schedule));
}

static void test_idle_does_not_duplicate_outstanding_request(void)
{
    pstvnc_h1_mpeg_calibration_rfb_gate_t gate;
    pstvnc_h1_mpeg_calibration_rfb_schedule_t schedule;
    pstvnc_h1_mpeg_calibration_rfb_request_t request;

    pstvnc_h1_mpeg_calibration_rfb_gate_init(&gate);
    pstvnc_h1_mpeg_calibration_rfb_schedule_init(&schedule);

    request = pstvnc_h1_mpeg_calibration_rfb_schedule_next(
        &schedule, &gate);
    assert(pstvnc_h1_mpeg_calibration_rfb_schedule_request_sent(
        &schedule, request));

    request = pstvnc_h1_mpeg_calibration_rfb_schedule_next(
        &schedule, &gate);
    assert(request == PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_HOLD);
    assert(!pstvnc_h1_mpeg_calibration_rfb_schedule_request_sent(
        &schedule, request));
}

static void test_freeze_with_request_in_flight(void)
{
    pstvnc_h1_mpeg_calibration_rfb_gate_t gate;
    pstvnc_h1_mpeg_calibration_rfb_schedule_t schedule;
    pstvnc_h1_mpeg_calibration_rfb_request_t request;

    pstvnc_h1_mpeg_calibration_rfb_gate_init(&gate);
    pstvnc_h1_mpeg_calibration_rfb_schedule_init(&schedule);

    request = pstvnc_h1_mpeg_calibration_rfb_schedule_next(
        &schedule, &gate);
    assert(pstvnc_h1_mpeg_calibration_rfb_schedule_request_sent(
        &schedule, request));

    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(&gate, 1));
    assert(!pstvnc_h1_mpeg_calibration_rfb_gate_allows_remote_present(&gate));

    /* Existing request remains protocol-owned until its response completes. */
    request = pstvnc_h1_mpeg_calibration_rfb_schedule_next(
        &schedule, &gate);
    assert(request == PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_HOLD);
    assert(pstvnc_h1_mpeg_calibration_rfb_schedule_update_complete(
        &schedule));
    assert(!pstvnc_h1_mpeg_calibration_rfb_schedule_has_outstanding_request(
        &schedule));

    /* Once complete, frozen calibration issues no replacement request. */
    request = pstvnc_h1_mpeg_calibration_rfb_schedule_next(
        &schedule, &gate);
    assert(request == PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_HOLD);
}

static void test_thaw_requires_exactly_one_full_refresh(void)
{
    pstvnc_h1_mpeg_calibration_rfb_gate_t gate;
    pstvnc_h1_mpeg_calibration_rfb_schedule_t schedule;
    pstvnc_h1_mpeg_calibration_rfb_request_t request;

    pstvnc_h1_mpeg_calibration_rfb_gate_init(&gate);
    pstvnc_h1_mpeg_calibration_rfb_schedule_init(&schedule);

    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(&gate, 1));
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(&gate, 0));

    request = pstvnc_h1_mpeg_calibration_rfb_schedule_next(
        &schedule, &gate);
    assert(request == PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_FULL);
    assert(pstvnc_h1_mpeg_calibration_rfb_schedule_request_sent(
        &schedule, request));

    /* IDLE while the full refresh is outstanding must not consume/send again. */
    request = pstvnc_h1_mpeg_calibration_rfb_schedule_next(
        &schedule, &gate);
    assert(request == PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_HOLD);

    assert(pstvnc_h1_mpeg_calibration_rfb_schedule_update_complete(
        &schedule));

    request = pstvnc_h1_mpeg_calibration_rfb_schedule_next(
        &schedule, &gate);
    assert(request == PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_INCREMENTAL);
}

static void test_full_refresh_obligation_survives_existing_request(void)
{
    pstvnc_h1_mpeg_calibration_rfb_gate_t gate;
    pstvnc_h1_mpeg_calibration_rfb_schedule_t schedule;
    pstvnc_h1_mpeg_calibration_rfb_request_t request;

    pstvnc_h1_mpeg_calibration_rfb_gate_init(&gate);
    pstvnc_h1_mpeg_calibration_rfb_schedule_init(&schedule);

    request = pstvnc_h1_mpeg_calibration_rfb_schedule_next(
        &schedule, &gate);
    assert(pstvnc_h1_mpeg_calibration_rfb_schedule_request_sent(
        &schedule, request));

    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(&gate, 1));
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(&gate, 0));

    /* Outstanding old request blocks scheduling but must not consume FULL. */
    request = pstvnc_h1_mpeg_calibration_rfb_schedule_next(
        &schedule, &gate);
    assert(request == PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_HOLD);

    assert(pstvnc_h1_mpeg_calibration_rfb_schedule_update_complete(
        &schedule));

    request = pstvnc_h1_mpeg_calibration_rfb_schedule_next(
        &schedule, &gate);
    assert(request == PSTVNC_H1_MPEG_CALIBRATION_RFB_REQUEST_FULL);
}

int main(void)
{
    test_ordinary_incremental_request();
    test_idle_does_not_duplicate_outstanding_request();
    test_freeze_with_request_in_flight();
    test_thaw_requires_exactly_one_full_refresh();
    test_full_refresh_obligation_survives_existing_request();

    puts("MPEG_CALIBRATION_RFB_SCHEDULE_HOST_TEST=PASS");
    return 0;
}

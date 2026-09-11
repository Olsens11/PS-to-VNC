/*
 * File synopsis:
 * Host-contract tests for calibration-driven RFB request/presentation quiesce.
 */

#include "h1_mpeg_calibration_rfb_gate.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    pstvnc_h1_mpeg_calibration_rfb_gate_t gate;

    pstvnc_h1_mpeg_calibration_rfb_gate_init(&gate);

    assert(pstvnc_h1_mpeg_calibration_rfb_gate_allows_request(&gate));
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_allows_remote_present(&gate));
    assert(!pstvnc_h1_mpeg_calibration_rfb_gate_take_full_refresh(&gate));

    /* Entry: stop new visual work without destroying protocol/session state. */
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(&gate, 1));
    assert(!pstvnc_h1_mpeg_calibration_rfb_gate_allows_request(&gate));
    assert(!pstvnc_h1_mpeg_calibration_rfb_gate_allows_remote_present(&gate));
    assert(!pstvnc_h1_mpeg_calibration_rfb_gate_take_full_refresh(&gate));

    /* Repeated frozen observations are idempotent. */
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(&gate, 1));
    assert(!pstvnc_h1_mpeg_calibration_rfb_gate_take_full_refresh(&gate));

    /* Thaw schedules exactly one nonincremental desktop resynchronization. */
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(&gate, 0));
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_allows_request(&gate));
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_allows_remote_present(&gate));
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_take_full_refresh(&gate));
    assert(!pstvnc_h1_mpeg_calibration_rfb_gate_take_full_refresh(&gate));

    /* Another complete ownership interval earns one new refresh. */
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(&gate, 1));
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(&gate, 0));
    assert(pstvnc_h1_mpeg_calibration_rfb_gate_take_full_refresh(&gate));
    assert(!pstvnc_h1_mpeg_calibration_rfb_gate_take_full_refresh(&gate));

    puts("MPEG_CALIBRATION_RFB_GATE_HOST_TEST=PASS");
    return 0;
}

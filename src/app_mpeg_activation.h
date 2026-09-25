/*
 * File synopsis:
 * Defines the A004 P10 trigger-agnostic Application ownership transfer from one
 * P9 ACCEPTED_PROTECTED calibration result into the existing R21 MPEG run-start
 * transaction.
 *
 * This module owns no generation counter and no lower mechanism. It copies the
 * exact P9 geometry, invokes R21 once with the same P2/P3 owners, independently
 * proves the resulting WAIT_FIRST_FRAME state, then either commits P9 without
 * thaw or uses P9's existing clean abort only when rollback is independently
 * proven.
 *
 * It does not choose a controller trigger, service/retire/reveal MPEG, activate
 * the Pi MPEG factory, recalibrate an active run, or modify ordinary app.c.
 *
 * Context:
 *   docs/ledge/LEDGE_FOREMAN_STATE.md,
 *   A004-APPLICATION-MPEG-PROTECTED-START-HANDOFF-P10.
 */

#ifndef PSTVNC_APP_MPEG_ACTIVATION_H
#define PSTVNC_APP_MPEG_ACTIVATION_H

#include "app_mpeg_calibration.h"
#include "app_mpeg_run.h"

typedef enum pstvnc_app_mpeg_activation_result {
    PSTVNC_APP_MPEG_ACTIVATION_OK = 0,
    PSTVNC_APP_MPEG_ACTIVATION_INVALID = -1,
    PSTVNC_APP_MPEG_ACTIVATION_ADMISSION_REJECTED = -2,
    PSTVNC_APP_MPEG_ACTIVATION_START_ROLLED_BACK = -3,
    PSTVNC_APP_MPEG_ACTIVATION_START_FAILURE_TEARDOWN_REQUIRED = -4,
    PSTVNC_APP_MPEG_ACTIVATION_SUCCESS_PROOF_TEARDOWN_REQUIRED = -5,
    PSTVNC_APP_MPEG_ACTIVATION_HANDOFF_COMMIT_TEARDOWN_REQUIRED = -6,
    PSTVNC_APP_MPEG_ACTIVATION_ROLLBACK_TEARDOWN_REQUIRED = -7
} pstvnc_app_mpeg_activation_result_t;

/*
 * Transfer one exact protected calibration into one existing R21 start attempt.
 *
 * Success leaves P9 IDLE, P2 continuously frozen and R21 as the sole
 * Application MPEG-run lifecycle owner in exact WAIT_FIRST_FRAME.
 *
 * A non-OK R21 result may use P9's existing abort/thaw path only when the run is
 * independently proven clean IDLE/current_generation zero and P3 is exact
 * RFB_ONLY/no snapshot. Any uncertain or irreversible result fault-contains P9
 * without thaw and returns an explicit teardown-required result.
 */
pstvnc_app_mpeg_activation_result_t
pstvnc_app_mpeg_activation_start_protected(
    pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_app_mpeg_run_t *run,
    const pstvnc_transport_access_t *transport_access,
    pstvnc_media_clock_t *media_clock);

#endif /* PSTVNC_APP_MPEG_ACTIVATION_H */

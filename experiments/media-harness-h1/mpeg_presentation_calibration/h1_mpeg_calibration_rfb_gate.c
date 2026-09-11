/*
 * File synopsis:
 * Implements the experiment-local safe-boundary RFB scheduling policy for MPEG
 * presentation calibration foreground ownership.
 */

#include "h1_mpeg_calibration_rfb_gate.h"

#include <stddef.h>
#include <string.h>

void pstvnc_h1_mpeg_calibration_rfb_gate_init(
    pstvnc_h1_mpeg_calibration_rfb_gate_t *gate)
{
    if (gate == NULL)
        return;

    memset(gate, 0, sizeof(*gate));
}

int pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(
    pstvnc_h1_mpeg_calibration_rfb_gate_t *gate,
    int frozen)
{
    unsigned new_frozen;

    if (gate == NULL)
        return 0;

    new_frozen = frozen ? 1u : 0u;

    if (gate->frozen && !new_frozen)
        gate->full_refresh_required = 1;

    gate->frozen = new_frozen;
    return 1;
}

int pstvnc_h1_mpeg_calibration_rfb_gate_allows_request(
    const pstvnc_h1_mpeg_calibration_rfb_gate_t *gate)
{
    return gate != NULL && !gate->frozen;
}

int pstvnc_h1_mpeg_calibration_rfb_gate_allows_remote_present(
    const pstvnc_h1_mpeg_calibration_rfb_gate_t *gate)
{
    return gate != NULL && !gate->frozen;
}

int pstvnc_h1_mpeg_calibration_rfb_gate_take_full_refresh(
    pstvnc_h1_mpeg_calibration_rfb_gate_t *gate)
{
    if (gate == NULL || gate->frozen || !gate->full_refresh_required)
        return 0;

    gate->full_refresh_required = 0;
    return 1;
}

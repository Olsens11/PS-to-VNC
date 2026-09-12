/*
 * File synopsis:
 * Supplies the neutral MPEG-calibration capability seam used only by the
 * historical CP2N visible-RFB + Issue-39 interaction build.
 *
 * The shared H1 interaction coordinator now contains later experiment-local
 * calibration hooks. CP2N predates that capability and is hardware-qualified
 * with ordinary Issue-39 controller routing only, so this file deliberately
 * keeps every calibration ownership/timing hook inert while satisfying the
 * coordinator's link contract. CP2O and later calibration-aware targets link
 * the real implementations instead.
 */

#include "h1_mpeg_calibration_entry_hold.h"
#include "h1_mpeg_calibration_interaction_binding.h"

#include <stddef.h>
#include <string.h>

void pstvnc_h1_mpeg_calibration_interaction_binding_init(
    pstvnc_h1_mpeg_calibration_interaction_binding_t *binding,
    int canvas_width,
    int canvas_height)
{
    (void)canvas_width;
    (void)canvas_height;

    if (binding != NULL)
        memset(binding, 0, sizeof(*binding));
}

int pstvnc_h1_mpeg_calibration_interaction_binding_service_controller(
    pstvnc_h1_mpeg_calibration_interaction_binding_t *binding,
    pstvnc_h1_mpeg_calibration_interaction_context_t *context,
    const pstvnc_controller_state_t *controller_state,
    int *consume_controller_state)
{
    (void)binding;
    (void)context;
    (void)controller_state;

    if (consume_controller_state != NULL)
        *consume_controller_state = 0;

    /* A disabled CP2N calibration owner must never be entered. */
    return 0;
}

int pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
    const pstvnc_h1_mpeg_calibration_interaction_binding_t *binding)
{
    (void)binding;
    return 0;
}

int pstvnc_h1_mpeg_calibration_interaction_binding_owns_mouse_suspension(
    const pstvnc_h1_mpeg_calibration_interaction_binding_t *binding)
{
    (void)binding;
    return 0;
}

const pstvnc_h1_rfb_flow_policy_t *
pstvnc_h1_mpeg_calibration_interaction_binding_rfb_policy(
    const pstvnc_h1_mpeg_calibration_interaction_binding_t *binding)
{
    (void)binding;
    return NULL;
}

void pstvnc_h1_mpeg_calibration_entry_hold_init(
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold)
{
    if (hold != NULL)
        memset(hold, 0, sizeof(*hold));
}

int pstvnc_h1_mpeg_calibration_entry_hold_observe(
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold,
    uint64_t now_us,
    int entry_available,
    int entry_chord_down,
    int *consume_controller_state)
{
    (void)now_us;
    (void)entry_available;
    (void)entry_chord_down;

    if (hold == NULL || consume_controller_state == NULL)
        return 0;

    hold->started_us = 0;
    hold->armed = 0;
    *consume_controller_state = 0;
    return 1;
}

int pstvnc_h1_mpeg_calibration_entry_hold_poll(
    pstvnc_h1_mpeg_calibration_entry_hold_t *hold,
    uint64_t now_us,
    int entry_available,
    int *activate_calibration)
{
    (void)now_us;
    (void)entry_available;

    if (hold == NULL || activate_calibration == NULL)
        return 0;

    hold->started_us = 0;
    hold->armed = 0;
    *activate_calibration = 0;
    return 1;
}

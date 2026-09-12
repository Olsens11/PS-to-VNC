/*
 * File synopsis:
 * Builds the live H1 interaction coordinator for CP2P by reusing its exact
 * through-Issue-39 implementation and redirecting only the two 750 ms
 * calibration-entry hold calls through the CP2P recalibration bridge.
 *
 * CP2O continues compiling h1_interaction_coordinator.c directly, so this file
 * cannot alter the qualified RFB+PCM baseline. The rest of the coordinator is
 * byte-for-byte the same source and retains the same ownership/routing rules.
 */
#include "mpeg_presentation_calibration/h1_mpeg_recalibration_entry_bridge.h"

#define pstvnc_h1_mpeg_calibration_entry_hold_observe \
    pstvnc_h1_mpeg_recalibration_entry_bridge_observe
#define pstvnc_h1_mpeg_calibration_entry_hold_poll \
    pstvnc_h1_mpeg_recalibration_entry_bridge_poll

#include "h1_interaction_coordinator.c"

#undef pstvnc_h1_mpeg_calibration_entry_hold_observe
#undef pstvnc_h1_mpeg_calibration_entry_hold_poll

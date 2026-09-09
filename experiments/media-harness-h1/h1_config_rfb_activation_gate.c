/*
 * File synopsis:
 * Opens the cumulative H1 experiment's first RFB hardware CONFIG path without
 * weakening the ordinary H1 validator or enabling any hybrid composition.
 *
 * The normal h1_config.c object is compiled in this one cumulative target with
 * pstvnc_h1_config_validate renamed to pstvnc_h1_config_validate_inner. This
 * wrapper retains that validator as authority for every existing non-RFB field.
 *
 * RFB OFF is passed through unchanged. RFB ON is accepted only when AUDIO and
 * MPEG are both OFF and the explicit CONFIG-v4 RFB queue/credit relationships
 * are internally valid. A normalized copy then clears only the RFB fields and
 * is submitted to the unchanged inner validator, proving all remaining profile
 * semantics through the same code used by qualified media sessions.
 *
 * This file is linked only by the cumulative H1 RFB-prep target. Other H1 builds
 * continue to expose the original public validator and therefore still reject
 * RFB ON.
 */

#include "h1_config.h"
#include "transport_protocol.h"

#include <string.h>

/* Renamed mechanically only in the cumulative target's h1_config.o. */
int pstvnc_h1_config_validate_inner(
    const pstvnc_h1_config_t *config);

static int h1_rfb_policy_valid(
    const pstvnc_h1_config_t *config)
{
    if (config->rfb_queue_capacity < config->max_data_payload ||
        config->rfb_initial_credit_bytes > config->rfb_queue_capacity ||
        config->rfb_credit_flush_on_empty > 1u ||
        config->rfb_credit_return_enabled > 1u)
        return 0;

    if (config->rfb_credit_return_enabled != 0u) {
        if (config->rfb_credit_batch_bytes == 0u ||
            config->rfb_credit_batch_bytes > config->rfb_queue_capacity)
            return 0;
    } else if (config->rfb_credit_batch_bytes != 0u) {
        /*
         * Keep an OFF return mechanism genuinely inert rather than carrying a
         * hidden dormant batch value into the first hardware authority.
         */
        return 0;
    }

    return 1;
}

int pstvnc_h1_config_validate(
    const pstvnc_h1_config_t *config)
{
    pstvnc_h1_config_t normalized;

    if (config == NULL)
        return 0;

    if (config->rfb_mode == PSTVNC_H1_RFB_OFF)
        return pstvnc_h1_config_validate_inner(config);

    if (config->rfb_mode != PSTVNC_H1_RFB_ON_RESERVED ||
        config->audio_mode != PSTVNC_H1_AUDIO_OFF ||
        config->video_mode != PSTVNC_H1_VIDEO_OFF ||
        !h1_rfb_policy_valid(config))
        return 0;

    memcpy(&normalized, config, sizeof(normalized));
    normalized.rfb_mode = PSTVNC_H1_RFB_OFF;
    normalized.rfb_queue_capacity = 0u;
    normalized.rfb_credit_batch_bytes = 0u;
    normalized.rfb_credit_flush_on_empty = 0u;
    normalized.rfb_credit_return_enabled = 0u;
    normalized.rfb_initial_credit_bytes = 0u;

    return pstvnc_h1_config_validate_inner(&normalized);
}

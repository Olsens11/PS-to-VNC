/*
 * File synopsis:
 * Opens exactly the CP2N-derived visible-RFB + existing PCM composition needed
 * by CP2O while keeping MPEG disabled and preserving the ordinary H1 validator
 * as authority for every non-RFB field.
 *
 * The cumulative target compiles h1_config.c with its public validator renamed
 * to pstvnc_h1_config_validate_inner(). RFB-only modes retain the already-proven
 * CP2J/CP2K/CP2L/CP2N policy. This descendant additionally permits PCM only
 * with visible RFB mode 2 and MPEG OFF. The normalized copy clears only RFB
 * fields before delegating to the unchanged inner validator, so the existing
 * PCM queue, credit, format, startup, and thread constraints remain authoritative.
 */

#include "h1_config.h"
#include "transport_protocol.h"

#include <string.h>

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

    if (!pstvnc_h1_rfb_mode_is_enabled(config->rfb_mode) ||
        config->video_mode != PSTVNC_H1_VIDEO_OFF ||
        !h1_rfb_policy_valid(config))
        return 0;

    /*
     * CP2O opens only one new hybrid: visible RFB + canonical PCM. Headless RFB
     * remains RFB-only, and compressed/reserved audio values stay rejected.
     */
    if (config->audio_mode != PSTVNC_H1_AUDIO_OFF &&
        (config->rfb_mode != PSTVNC_H1_RFB_ON_VISIBLE ||
         config->audio_mode != PSTVNC_H1_AUDIO_PCM))
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

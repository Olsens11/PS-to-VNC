/*
 * File synopsis:
 * Opens exactly CP2P item #10's visible-RFB + MPEG2 + optional canonical-PCM
 * composition while preserving the ordinary H1 validator as authority for all
 * non-RFB media, queue, credit, decoder, scheduler, and thread constraints.
 *
 * The CP2P target compiles h1_config.c with its public validator renamed to
 * pstvnc_h1_config_validate_inner(). This wrapper validates CP2P's RFB policy,
 * clears only the RFB fields in a normalized copy, and delegates every MPEG/PCM
 * relationship to that unchanged inner validator.
 */
#include "h1_config.h"
#include "transport_protocol.h"

#include <string.h>

int pstvnc_h1_config_validate_inner(
    const pstvnc_h1_config_t *config);

static int h1_cp2p_rfb_policy_valid(
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

    if (config->rfb_mode != PSTVNC_H1_RFB_ON_VISIBLE ||
        config->video_mode != PSTVNC_H1_VIDEO_MPEG2_ES ||
        (config->audio_mode != PSTVNC_H1_AUDIO_OFF &&
         config->audio_mode != PSTVNC_H1_AUDIO_PCM) ||
        !h1_cp2p_rfb_policy_valid(config))
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

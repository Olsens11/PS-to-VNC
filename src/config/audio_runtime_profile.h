/*
 * File synopsis:
 * Defines Configuration's immutable selected AUDIO runtime profile as three
 * distinct existing owner-value groups: Transport channel policy, PCM playback
 * format/volume, and AUDIO session worker/reservoir timing values. R36 adds no
 * media-clock offset and activates no runtime mechanism.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md;
 * docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A002-AUDIO-RUNTIME-PROFILE-AUTHORITY-R36.
 */

#ifndef PSTVNC_CONFIG_AUDIO_RUNTIME_PROFILE_H
#define PSTVNC_CONFIG_AUDIO_RUNTIME_PROFILE_H

#include "audio/session.h"
#include "profile.h"
#include "transport/transport.h"

typedef struct pstvnc_config_audio_runtime_profile {
    pstvnc_transport_audio_channel_config_t transport;
    pstvnc_config_pcm_profile_t pcm;
    pstvnc_audio_session_values_t session;
} pstvnc_config_audio_runtime_profile_t;

int pstvnc_config_audio_runtime_profile_valid(
    const pstvnc_config_audio_runtime_profile_t *profile);

/*
 * Copy the selected immutable profile into caller-owned storage.
 * Mutating that copy cannot mutate Configuration-owned authority.
 */
int pstvnc_config_audio_runtime_profile_selected(
    pstvnc_config_audio_runtime_profile_t *profile);

#endif /* PSTVNC_CONFIG_AUDIO_RUNTIME_PROFILE_H */

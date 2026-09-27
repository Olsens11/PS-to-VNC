/*
 * File synopsis:
 * Declares the top-level coordinator entry points while keeping concrete
 * Transport configuration authority outside application implementation.
 *
 * Context: docs/CLEAN_ARCHITECTURE.md, "Application coordinator";
 * docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md.
 */

#ifndef PSTVNC_APP_H
#define PSTVNC_APP_H

#include "config/audio_runtime_profile.h"
#include "config/profile.h"
#include "transport/transport.h"

/*
 * Run the ordinary session lifecycle with already-selected owner values.
 *
 * R42 forwards the exact RFB, AUDIO and MPEG Transport values into one
 * Transport runtime, creates one fresh unarmed media clock and one fresh R41
 * AUDIO coordinator for each accepted physical Wire Session. AUDIO remains
 * dormant until the first genuinely started protected MPEG run.
 */
int pstvnc_app_run_with_session_profiles(
    const pstvnc_transport_session_config_t *transport_config,
    const pstvnc_config_audio_runtime_profile_t *audio_profile,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_transport_config,
    const pstvnc_config_media_clock_profile_t *media_clock_profile);

/*
 * Product entry resolves all selected RFB/AUDIO/MPEG/media-clock authority
 * before platform startup, then enters the same ordinary session lifecycle.
 */
int pstvnc_app_run(void);

#endif

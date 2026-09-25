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

#include "config/profile.h"
#include "transport/transport.h"

/*
 * Run the ordinary session lifecycle with already-selected owner values.
 *
 * R27 forwards the exact RFB and MPEG Transport values into one Transport
 * runtime and creates one fresh unarmed media clock for each accepted physical
 * Wire Session. This seam does not start an MPEG run or arm the media clock.
 */
int pstvnc_app_run_with_session_profiles(
    const pstvnc_transport_session_config_t *transport_config,
    const pstvnc_transport_mpeg_channel_config_t *mpeg_transport_config,
    const pstvnc_config_media_clock_profile_t *media_clock_profile);

/*
 * Product entry resolves all selected RFB/MPEG/media-clock authority before
 * platform startup, then enters the same ordinary session lifecycle.
 */
int pstvnc_app_run(void);

#endif

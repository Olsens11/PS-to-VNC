/*
 * File synopsis:
 * Projects Configuration's one canonical current RFB runtime profile onto the
 * already-owned PS2 Transport session-config boundary.
 *
 * The selected numeric values are generated from rfb_runtime_profile.json.
 * This boundary owns no Wire Session identity, does not open Transport, and
 * does not activate Application RFB. Semantic OFF deliberately yields no
 * running Transport projection instead of manufacturing a zero-valued runtime.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A003-RFB-SHARED-RUNTIME-PROFILE-R14.
 */

#ifndef PSTVNC_CONFIG_RFB_RUNTIME_PROFILE_H
#define PSTVNC_CONFIG_RFB_RUNTIME_PROFILE_H

#include "profile.h"
#include "transport/transport.h"

pstvnc_config_rfb_mode_t pstvnc_config_rfb_runtime_profile_selected_mode(void);

int pstvnc_config_rfb_runtime_profile_project(
    pstvnc_config_rfb_mode_t mode,
    pstvnc_transport_session_config_t *transport_config);

int pstvnc_config_rfb_runtime_profile_selected(
    pstvnc_transport_session_config_t *transport_config);

#endif /* PSTVNC_CONFIG_RFB_RUNTIME_PROFILE_H */

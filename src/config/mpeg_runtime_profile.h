/*
 * File synopsis:
 * Defines Configuration's one selected product MPEG runtime profile. The value
 * aggregates the already-owned narrow Transport, MPEG decoder/worker/PS2
 * execution, and Display scheduler profile types without copying or renaming
 * their fields.
 *
 * This is internal product configuration authority, not the PSTV CONFIG wire.
 * It selects mechanism inputs only; it does not own active MPEG geometry,
 * generation identity, lifecycle ordering, Transport session establishment,
 * decoder/worker startup, Presentation activation, or Application orchestration.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A003-MPEG-RUNTIME-PROFILE-AUTHORITY-R7.
 */

#ifndef PSTVNC_CONFIG_MPEG_RUNTIME_PROFILE_H
#define PSTVNC_CONFIG_MPEG_RUNTIME_PROFILE_H

#include "display/mpeg_scheduler.h"
#include "mpeg/decoder.h"
#include "mpeg/ps2_worker_runtime.h"
#include "mpeg/worker.h"
#include "transport/transport.h"

typedef struct pstvnc_config_mpeg_runtime_profile {
    pstvnc_transport_mpeg_channel_config_t transport;
    pstvnc_mpeg_decoder_config_t decoder;
    pstvnc_mpeg_worker_values_t worker;
    pstvnc_mpeg_ps2_worker_runtime_values_t ps2_worker_runtime;
    pstvnc_mpeg_scheduler_profile_t scheduler;
} pstvnc_config_mpeg_runtime_profile_t;

/*
 * Return the single selected current product profile.
 *
 * The returned object has static const lifetime. Callers may copy any narrow
 * owner value and pass that copy explicitly to its mechanism owner; no caller
 * receives mutable Configuration-owned profile storage.
 */
const pstvnc_config_mpeg_runtime_profile_t *
pstvnc_config_mpeg_runtime_profile_selected(void);

#endif /* PSTVNC_CONFIG_MPEG_RUNTIME_PROFILE_H */

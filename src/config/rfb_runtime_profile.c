/*
 * File synopsis:
 * Publishes the PS2 projection of the canonical current RFB runtime profile.
 * All selected numeric values come from the deterministic generated header;
 * this source contains no independent tuning defaults and no session identity.
 *
 * RFB OFF is an absent composition projection. The module performs no socket
 * work, Transport allocation/start, RFB parsing, recovery, or Application
 * activation.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A003-RFB-SHARED-RUNTIME-PROFILE-R14.
 */

#include "rfb_runtime_profile.h"
#include "rfb_runtime_profile_generated.h"

pstvnc_config_rfb_mode_t pstvnc_config_rfb_runtime_profile_selected_mode(void)
{
    return PSTVNC_CONFIG_RFB_SELECTED_MODE_ON ?
        PSTVNC_CONFIG_RFB_ON :
        PSTVNC_CONFIG_RFB_OFF;
}

int pstvnc_config_rfb_runtime_profile_project(
    pstvnc_config_rfb_mode_t mode,
    pstvnc_transport_session_config_t *transport_config)
{
    pstvnc_transport_session_config_t candidate;

    if (transport_config == NULL || mode != PSTVNC_CONFIG_RFB_ON)
        return 0;

    candidate.rfb_queue_capacity = PSTVNC_CONFIG_RFB_WINDOW_BYTES;
    candidate.rfb_initial_credit_bytes = PSTVNC_CONFIG_RFB_WINDOW_BYTES;
    candidate.rfb_credit_batch_bytes = PSTVNC_CONFIG_RFB_CREDIT_BATCH_BYTES;
    candidate.rfb_credit_flush_on_empty =
        PSTVNC_CONFIG_RFB_CREDIT_FLUSH_ON_EMPTY;
    candidate.rfb_credit_return_enabled =
        PSTVNC_CONFIG_RFB_CREDIT_RETURN_ENABLED;
    candidate.receiver_thread_stack_size =
        PSTVNC_CONFIG_RFB_RECEIVER_THREAD_STACK_SIZE;
    candidate.receiver_thread_priority =
        PSTVNC_CONFIG_RFB_RECEIVER_THREAD_PRIORITY;
    candidate.max_data_payload = PSTVNC_CONFIG_RFB_MAX_DATA_PAYLOAD;

    *transport_config = candidate;
    return 1;
}

int pstvnc_config_rfb_runtime_profile_selected(
    pstvnc_transport_session_config_t *transport_config)
{
    return pstvnc_config_rfb_runtime_profile_project(
        pstvnc_config_rfb_runtime_profile_selected_mode(),
        transport_config);
}

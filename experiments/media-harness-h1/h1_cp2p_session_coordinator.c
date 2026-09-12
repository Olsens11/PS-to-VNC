/*
 * File synopsis:
 * Implements CP2P's one session-level lifecycle coordinator.
 *
 * A calibration accepted edge is consumed exactly once, converted into one
 * immutable generation by prepare_start(), and serialized as one START on the
 * existing PSTV socket using the transport-owned session id.
 */
#include "h1_cp2p_session_coordinator.h"

#include "h1_config.h"
#include "mpeg_presentation_calibration/h1_mpeg_start_transport.h"

const pstvnc_h1_config_t *pstvnc_h1_transport_config(
    const struct pstvnc_h1_transport_runtime *runtime);

#include <stddef.h>
#include <string.h>

static int h1_cp2p_session_calibration_entry_gate(
    void *context,
    int *enter_now)
{
    pstvnc_h1_cp2p_session_coordinator_t *coordinator =
        (pstvnc_h1_cp2p_session_coordinator_t *)context;
    pstvnc_h1_mpeg_recalibration_begin_result_t begin_result;
    pstvnc_mpeg_calibration_t *calibration;

    if (coordinator == NULL || !coordinator->initialized ||
        enter_now == NULL)
        return 0;

    if (coordinator->recalibration.awaiting_restored_full_rfb) {
        *enter_now = pstvnc_h1_mpeg_recalibration_take_entry_ready(
            &coordinator->recalibration,
            &coordinator->rfb_flow)
            ? 1
            : 0;
        return 1;
    }

    calibration = &coordinator->interaction.mpeg_calibration.runtime
        .foreground.adapter.calibration;

    if (!pstvnc_h1_mpeg_recalibration_begin(
            &coordinator->recalibration,
            &coordinator->mpeg_handoff,
            calibration,
            &coordinator->rfb_flow,
            coordinator->clear_mpeg,
            coordinator->clear_mpeg_context,
            &begin_result))
        return 0;

    if (begin_result == PSTVNC_H1_MPEG_RECALIBRATION_ENTER_NOW) {
        *enter_now = 1;
        return 1;
    }

    if (begin_result != PSTVNC_H1_MPEG_RECALIBRATION_WAIT_FOR_FULL_RFB)
        return 0;

    coordinator->current_start_contract_valid = 0;
    *enter_now = 0;
    return 1;
}

static int h1_cp2p_session_start_accepted_calibration(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator,
    const pstvnc_mpeg_cal_region_t *committed_region)
{
    pstvnc_h1_mpeg_start_contract_t contract;

    if (coordinator == NULL || committed_region == NULL ||
        !coordinator->initialized || coordinator->transport == NULL ||
        pstvnc_h1_mpeg_presentation_owner_state(
            &coordinator->mpeg_handoff.owner) !=
            PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY)
        return 0;

    memset(&contract, 0, sizeof(contract));
    if (!pstvnc_h1_mpeg_start_handoff_prepare_start(
            &coordinator->mpeg_handoff,
            committed_region,
            &contract))
        return 0;

    if (!pstvnc_h1_mpeg_start_transport_send(
            coordinator->transport,
            coordinator->session_id,
            &contract)) {
        (void)pstvnc_h1_mpeg_start_handoff_abort_start(
            &coordinator->mpeg_handoff,
            contract.generation);
        coordinator->current_start_contract_valid = 0;
        return 0;
    }

    if (coordinator->start_messages_sent == UINT32_MAX) {
        (void)pstvnc_h1_mpeg_start_handoff_abort_start(
            &coordinator->mpeg_handoff,
            contract.generation);
        return 0;
    }

    coordinator->current_start_contract = contract;
    coordinator->current_start_contract_valid = 1;
    coordinator->start_messages_sent++;
    return 1;
}

int pstvnc_h1_cp2p_session_coordinator_init(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator,
    struct pstvnc_h1_transport_runtime *transport,
    pstvnc_h1_mpeg_recalibration_clear_mpeg_fn clear_mpeg,
    void *clear_mpeg_context)
{
    const pstvnc_h1_config_t *config;

    if (coordinator == NULL || transport == NULL)
        return 0;

    config = pstvnc_h1_transport_config(transport);
    if (config == NULL)
        return 0;

    memset(coordinator, 0, sizeof(*coordinator));
    pstvnc_h1_interaction_coordinator_init(&coordinator->interaction);
    pstvnc_h1_mpeg_start_handoff_init(
        &coordinator->mpeg_handoff,
        PSTVNC_DISPLAY_WIDTH,
        PSTVNC_DISPLAY_HEIGHT);
    pstvnc_h1_mpeg_recalibration_init(&coordinator->recalibration);
    pstvnc_h1_mpeg_cp2p_rfb_flow_init(
        &coordinator->rfb_flow,
        &coordinator->interaction.mpeg_calibration.runtime,
        &coordinator->mpeg_handoff.owner);

    if (!pstvnc_h1_mpeg_cp2p_rfb_flow_prepare_policy(
            &coordinator->rfb_flow,
            &coordinator->rfb_policy))
        return 0;

    coordinator->transport = transport;
    coordinator->session_id = config->session_id;
    coordinator->clear_mpeg = clear_mpeg;
    coordinator->clear_mpeg_context = clear_mpeg_context;
    coordinator->initialized = 1;

    if (!pstvnc_h1_interaction_coordinator_set_calibration_entry_gate(
            &coordinator->interaction,
            h1_cp2p_session_calibration_entry_gate,
            coordinator)) {
        coordinator->initialized = 0;
        return 0;
    }

    return 1;
}

int pstvnc_h1_cp2p_session_coordinator_present(
    void *context,
    const pstvnc_framebuffer_t *framebuffer)
{
    pstvnc_h1_cp2p_session_coordinator_t *coordinator =
        (pstvnc_h1_cp2p_session_coordinator_t *)context;

    if (coordinator == NULL || !coordinator->initialized)
        return 0;

    return pstvnc_h1_interaction_coordinator_present(
        &coordinator->interaction,
        framebuffer);
}

int pstvnc_h1_cp2p_session_coordinator_service(
    void *context,
    pstvnc_rfb_session_t *session)
{
    pstvnc_h1_cp2p_session_coordinator_t *coordinator =
        (pstvnc_h1_cp2p_session_coordinator_t *)context;
    pstvnc_mpeg_cal_region_t accepted_region;

    if (coordinator == NULL || !coordinator->initialized)
        return 0;

    if (!pstvnc_h1_interaction_coordinator_service(
            &coordinator->interaction,
            session))
        return 0;

    if (!pstvnc_h1_interaction_coordinator_take_calibration_accept(
            &coordinator->interaction,
            &accepted_region))
        return 1;

    return h1_cp2p_session_start_accepted_calibration(
        coordinator,
        &accepted_region);
}

const pstvnc_h1_rfb_flow_policy_t *
pstvnc_h1_cp2p_session_coordinator_rfb_policy(
    const pstvnc_h1_cp2p_session_coordinator_t *coordinator)
{
    if (coordinator == NULL || !coordinator->initialized)
        return NULL;
    return &coordinator->rfb_policy;
}

pstvnc_h1_mpeg_start_handoff_t *
pstvnc_h1_cp2p_session_coordinator_mpeg_handoff(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator)
{
    if (coordinator == NULL || !coordinator->initialized)
        return NULL;
    return &coordinator->mpeg_handoff;
}

const pstvnc_h1_mpeg_start_contract_t *
pstvnc_h1_cp2p_session_coordinator_current_start_contract(
    const pstvnc_h1_cp2p_session_coordinator_t *coordinator)
{
    pstvnc_h1_mpeg_presentation_owner_state_t owner_state;

    if (coordinator == NULL || !coordinator->initialized ||
        !coordinator->current_start_contract_valid)
        return NULL;

    owner_state = pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator->mpeg_handoff.owner);
    if ((owner_state != PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME &&
         owner_state != PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED) ||
        coordinator->mpeg_handoff.owner.generation !=
            coordinator->current_start_contract.generation)
        return NULL;

    return &coordinator->current_start_contract;
}

int pstvnc_h1_cp2p_session_coordinator_shutdown(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator)
{
    int result;

    if (coordinator == NULL || !coordinator->initialized)
        return -1;

    result = pstvnc_h1_interaction_coordinator_shutdown(
        &coordinator->interaction);
    coordinator->initialized = 0;
    coordinator->transport = NULL;
    coordinator->clear_mpeg = NULL;
    coordinator->clear_mpeg_context = NULL;
    coordinator->current_start_contract_valid = 0;
    return result;
}

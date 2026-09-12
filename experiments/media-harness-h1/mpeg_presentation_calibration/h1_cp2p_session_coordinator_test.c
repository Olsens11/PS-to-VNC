/*
 * Host contract for the CP2P session coordinator.
 */
#include "../h1_cp2p_session_coordinator.h"
#include "../h1_config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned int send_count;
static int send_success = 1;
static uint32_t sent_session_id;
static pstvnc_h1_mpeg_start_contract_t sent_contract;
static unsigned int clear_count;
static uint32_t cleared_generation;
static unsigned int retire_begin_count;
static uint32_t retire_generation;
static int retire_poll_result;
static pstvnc_h1_config_t fake_config;

void pstvnc_h1_interaction_coordinator_init(
    pstvnc_h1_interaction_coordinator_t *coordinator)
{
    memset(coordinator, 0, sizeof(*coordinator));
    pstvnc_h1_mpeg_calibration_runtime_init(
        &coordinator->mpeg_calibration.runtime,
        PSTVNC_DISPLAY_WIDTH,
        PSTVNC_DISPLAY_HEIGHT);
}

int pstvnc_h1_interaction_coordinator_set_calibration_entry_gate(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_h1_interaction_calibration_entry_gate_fn gate,
    void *gate_context)
{
    if (coordinator == NULL)
        return 0;
    coordinator->calibration_entry_gate = gate;
    coordinator->calibration_entry_gate_context = gate_context;
    return 1;
}

int pstvnc_h1_interaction_coordinator_take_calibration_accept(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_mpeg_cal_region_t *region)
{
    if (coordinator == NULL || region == NULL ||
        !coordinator->accepted_calibration_pending)
        return 0;
    *region = coordinator->accepted_calibration_region;
    coordinator->accepted_calibration_pending = 0;
    return 1;
}

int pstvnc_h1_interaction_coordinator_present(
    void *context,
    const pstvnc_framebuffer_t *framebuffer)
{
    (void)context;
    (void)framebuffer;
    return 1;
}

int pstvnc_h1_interaction_coordinator_service(
    void *context,
    pstvnc_rfb_session_t *session)
{
    (void)context;
    (void)session;
    return 1;
}

int pstvnc_h1_interaction_coordinator_shutdown(
    pstvnc_h1_interaction_coordinator_t *coordinator)
{
    return coordinator != NULL ? 0 : -1;
}

const pstvnc_h1_config_t *pstvnc_h1_transport_config(
    const struct pstvnc_h1_transport_runtime *runtime)
{
    return runtime != NULL ? &fake_config : NULL;
}

int pstvnc_h1_mpeg_start_transport_send(
    struct pstvnc_h1_transport_runtime *runtime,
    uint32_t session_id,
    const pstvnc_h1_mpeg_start_contract_t *contract)
{
    (void)runtime;
    if (contract == NULL)
        return 0;
    send_count++;
    sent_session_id = session_id;
    sent_contract = *contract;
    return send_success;
}

int pstvnc_h1_transport_mpeg_retire_begin(
    struct pstvnc_h1_transport_runtime *runtime,
    uint32_t generation)
{
    (void)runtime;
    if (generation == 0u || retire_begin_count != 0u)
        return 0;
    retire_begin_count++;
    retire_generation = generation;
    return 1;
}

int pstvnc_h1_transport_mpeg_retire_poll(
    struct pstvnc_h1_transport_runtime *runtime,
    uint32_t generation)
{
    (void)runtime;
    if (generation == 0u || generation != retire_generation)
        return -1;
    return retire_poll_result;
}

static int clear_mpeg(void *context, uint32_t generation)
{
    (void)context;
    clear_count++;
    cleared_generation = generation;
    return 1;
}

static pstvnc_mpeg_cal_region_t sample_region(int x, int y)
{
    pstvnc_mpeg_cal_region_t region;
    memset(&region, 0, sizeof(region));
    region.x = x;
    region.y = y;
    region.width = 320;
    region.height = 240;
    region.inner_matte_x = 8;
    region.inner_matte_y = 4;
    region.outer_matte_x = 1;
    region.outer_matte_y = 1;
    return region;
}

int main(void)
{
    static pstvnc_h1_cp2p_session_coordinator_t coordinator;
    struct pstvnc_h1_transport_runtime *transport =
        (struct pstvnc_h1_transport_runtime *)(uintptr_t)1;
    pstvnc_rfb_session_t rfb_session;
    pstvnc_mpeg_cal_region_t region;
    pstvnc_h1_mpeg_start_contract_t active_contract;
    pstvnc_h1_rfb_request_policy_decision_t decision;
    int enter_now = 0;

    memset(&rfb_session, 0, sizeof(rfb_session));
    memset(&fake_config, 0, sizeof(fake_config));
    fake_config.session_id = UINT32_C(0x1234abcd);

    assert(pstvnc_h1_cp2p_session_coordinator_init(
        &coordinator, transport, clear_mpeg, NULL));

    region = sample_region(64, 48);
    coordinator.interaction.accepted_calibration_region = region;
    coordinator.interaction.accepted_calibration_pending = 1;

    assert(pstvnc_h1_cp2p_session_coordinator_service(
        &coordinator, &rfb_session));
    assert(send_count == 1u);
    assert(sent_session_id == UINT32_C(0x1234abcd));
    assert(sent_contract.draw_x == region.x);
    assert(sent_contract.draw_y == region.y);
    assert(sent_contract.draw_width == region.width);
    assert(sent_contract.draw_height == region.height);
    assert(sent_contract.inner_matte_x == region.inner_matte_x);
    assert(sent_contract.inner_matte_y == region.inner_matte_y);
    assert(sent_contract.generation != 0u);
    assert(pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator.mpeg_handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_WAIT_FIRST_FRAME);

    assert(pstvnc_h1_cp2p_session_coordinator_service(
        &coordinator, &rfb_session));
    assert(send_count == 1u);

    assert(pstvnc_h1_mpeg_start_handoff_abort_start(
        &coordinator.mpeg_handoff, sent_contract.generation));
    coordinator.current_start_contract_valid = 0;

    region = sample_region(96, 64);
    coordinator.interaction.accepted_calibration_region = region;
    coordinator.interaction.accepted_calibration_pending = 1;
    send_success = 0;
    assert(!pstvnc_h1_cp2p_session_coordinator_service(
        &coordinator, &rfb_session));
    assert(send_count == 2u);
    assert(pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator.mpeg_handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);
    assert(!coordinator.current_start_contract_valid);

    /* Saturated diagnostics cannot put an unaccounted START on the wire. */
    send_success = 1;
    coordinator.start_messages_sent = UINT32_MAX;
    region = sample_region(112, 72);
    coordinator.interaction.accepted_calibration_region = region;
    coordinator.interaction.accepted_calibration_pending = 1;
    assert(!pstvnc_h1_cp2p_session_coordinator_service(
        &coordinator, &rfb_session));
    assert(send_count == 2u);
    assert(pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator.mpeg_handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);
    coordinator.start_messages_sent = 2u;

    region = sample_region(128, 80);
    assert(pstvnc_h1_mpeg_start_handoff_prepare_start(
        &coordinator.mpeg_handoff, &region, &active_contract));
    assert(pstvnc_h1_mpeg_start_handoff_first_frame_presented(
        &coordinator.mpeg_handoff, active_contract.generation));
    coordinator.current_start_contract = active_contract;
    coordinator.current_start_contract_valid = 1;

    /* First gate pass requests exact Pi retirement only. */
    assert(coordinator.interaction.calibration_entry_gate(
        coordinator.interaction.calibration_entry_gate_context,
        &enter_now));
    assert(!enter_now);
    assert(retire_begin_count == 1u);
    assert(retire_generation == active_contract.generation);
    assert(coordinator.pi_retire_pending);
    assert(coordinator.pi_retire_generation == active_contract.generation);
    assert(clear_count == 0u);
    assert(pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator.mpeg_handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED);
    assert(coordinator.current_start_contract_valid);

    /* Deferred polling cannot clear local ownership before exact Pi ACK. */
    retire_poll_result = 0;
    enter_now = 0;
    assert(coordinator.interaction.calibration_entry_gate(
        coordinator.interaction.calibration_entry_gate_context,
        &enter_now));
    assert(!enter_now);
    assert(retire_begin_count == 1u);
    assert(clear_count == 0u);
    assert(pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator.mpeg_handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED);

    /* Exact Pi completion unlocks the existing local retire/restoration path. */
    retire_poll_result = 1;
    enter_now = 0;
    assert(coordinator.interaction.calibration_entry_gate(
        coordinator.interaction.calibration_entry_gate_context,
        &enter_now));
    assert(!enter_now);
    assert(!coordinator.pi_retire_pending);
    assert(coordinator.pi_retire_generation == 0u);
    assert(clear_count == 1u);
    assert(cleared_generation == active_contract.generation);
    assert(pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator.mpeg_handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_RFB_ONLY);
    assert(!coordinator.current_start_contract_valid);
    assert(coordinator.interaction.mpeg_calibration.runtime.foreground.adapter
        .calibration.has_committed);
    assert(coordinator.interaction.mpeg_calibration.runtime.foreground.adapter
        .calibration.committed.x == region.x);
    assert(coordinator.interaction.mpeg_calibration.runtime.foreground.adapter
        .calibration.committed.outer_matte_x == region.outer_matte_x);

    decision = coordinator.rfb_policy.next_request(
        coordinator.rfb_policy.context);
    assert(decision == PSTVNC_H1_RFB_REQUEST_POLICY_FULL);
    assert(coordinator.rfb_policy.request_sent(
        coordinator.rfb_policy.context, decision));
    assert(coordinator.rfb_policy.update_complete(
        coordinator.rfb_policy.context));

    enter_now = 0;
    assert(coordinator.interaction.calibration_entry_gate(
        coordinator.interaction.calibration_entry_gate_context,
        &enter_now));
    assert(enter_now);

    assert(pstvnc_h1_cp2p_session_coordinator_shutdown(&coordinator) == 0);
    puts("H1_CP2P_SESSION_COORDINATOR_HOST_TEST=PASS");
    return 0;
}

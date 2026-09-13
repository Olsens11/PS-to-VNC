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
static unsigned int stop_request_count;
static unsigned int stop_poll_count;
static int stop_poll_result;
static uint32_t stop_generation;
static unsigned int stop_request_order;
static unsigned int stop_complete_order;
static unsigned int retire_begin_order;
static unsigned int retire_begin_count;
static uint32_t retire_generation;
static int retire_poll_result;
static unsigned int generation_open_count;
static unsigned int generation_abort_count;
static unsigned int retire_finalize_count;
static uint32_t transport_generation;
static uint32_t finalized_generation;
static uint32_t finalized_discarded;
static unsigned int lifecycle_order;
static unsigned int clear_order;
static unsigned int finalize_order;
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

int pstvnc_h1_transport_mpeg_generation_open(
    struct pstvnc_h1_transport_runtime *runtime,
    uint32_t generation)
{
    (void)runtime;
    if (generation == 0u || transport_generation != 0u)
        return 0;
    transport_generation = generation;
    generation_open_count++;
    return 1;
}

int pstvnc_h1_transport_mpeg_generation_abort(
    struct pstvnc_h1_transport_runtime *runtime,
    uint32_t generation)
{
    (void)runtime;
    if (generation == 0u || transport_generation != generation)
        return 0;
    transport_generation = 0u;
    generation_abort_count++;
    return 1;
}

int pstvnc_h1_transport_mpeg_retire_begin(
    struct pstvnc_h1_transport_runtime *runtime,
    uint32_t generation)
{
    (void)runtime;
    if (generation == 0u || retire_begin_count != 0u ||
        transport_generation != generation)
        return 0;
    retire_begin_count++;
    retire_generation = generation;
    retire_begin_order = ++lifecycle_order;
    return 1;
}

int pstvnc_h1_transport_mpeg_retire_poll(
    struct pstvnc_h1_transport_runtime *runtime,
    uint32_t generation)
{
    (void)runtime;
    if (generation == 0u || generation != retire_generation)
        return -1;
    if (retire_poll_result == 1)
        transport_generation = 0u;
    return retire_poll_result;
}

int pstvnc_h1_transport_mpeg_retire_finalize(
    struct pstvnc_h1_transport_runtime *runtime,
    uint32_t generation,
    uint32_t *bytes_discarded)
{
    (void)runtime;
    if (generation == 0u || generation != retire_generation ||
        transport_generation != 0u || clear_count == 0u)
        return 0;
    retire_finalize_count++;
    finalized_generation = generation;
    finalized_discarded = 17u;
    finalize_order = ++lifecycle_order;
    if (bytes_discarded != NULL)
        *bytes_discarded = finalized_discarded;
    retire_generation = 0u;
    return 1;
}


static int arm_mpeg(
    void *context,
    pstvnc_h1_mpeg_start_handoff_t *handoff,
    const pstvnc_h1_mpeg_start_contract_t *contract)
{
    (void)context;
    return handoff != NULL && contract != NULL;
}

static int request_mpeg_stop(void *context, uint32_t generation)
{
    (void)context;

    if (generation == 0u || stop_request_count != 0u)
        return 0;

    stop_request_count++;
    stop_generation = generation;
    stop_request_order = ++lifecycle_order;
    return 1;
}

static int poll_mpeg_stop(void *context, uint32_t generation)
{
    (void)context;

    if (generation == 0u || generation != stop_generation)
        return -1;

    stop_poll_count++;

    if (stop_poll_result == 1 && stop_complete_order == 0u)
        stop_complete_order = ++lifecycle_order;

    return stop_poll_result;
}

static int clear_mpeg(void *context, uint32_t generation)
{
    (void)context;
    clear_count++;
    cleared_generation = generation;
    clear_order = ++lifecycle_order;
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
    assert(generation_open_count == 1u);
    assert(transport_generation == sent_contract.generation);
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
    assert(pstvnc_h1_transport_mpeg_generation_abort(
        transport, sent_contract.generation));
    assert(generation_abort_count == 1u);
    coordinator.current_start_contract_valid = 0;

    region = sample_region(96, 64);
    coordinator.interaction.accepted_calibration_region = region;
    coordinator.interaction.accepted_calibration_pending = 1;
    send_success = 0;
    assert(!pstvnc_h1_cp2p_session_coordinator_service(
        &coordinator, &rfb_session));
    assert(send_count == 2u);
    assert(generation_open_count == 2u);
    assert(generation_abort_count == 2u);
    assert(transport_generation == 0u);
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

    assert(pstvnc_h1_cp2p_session_coordinator_set_mpeg_worker(
        &coordinator,
        arm_mpeg,
        request_mpeg_stop,
        poll_mpeg_stop,
        NULL));

    region = sample_region(128, 80);
    assert(pstvnc_h1_mpeg_start_handoff_prepare_start(
        &coordinator.mpeg_handoff, &region, &active_contract));
    assert(pstvnc_h1_mpeg_start_handoff_first_frame_presented(
        &coordinator.mpeg_handoff, active_contract.generation));
    assert(pstvnc_h1_transport_mpeg_generation_open(
        transport, active_contract.generation));
    assert(generation_open_count == 3u);
    coordinator.current_start_contract = active_contract;
    coordinator.current_start_contract_valid = 1;

    /*
     * First gate pass requests only the local decoder stop. The Pi generation
     * must remain live until the worker reaches a completed-picture boundary.
     */
    assert(coordinator.interaction.calibration_entry_gate(
        coordinator.interaction.calibration_entry_gate_context,
        &enter_now));
    assert(!enter_now);
    assert(stop_request_count == 1u);
    assert(stop_generation == active_contract.generation);
    assert(coordinator.mpeg_stop_pending);
    assert(coordinator.mpeg_stop_generation == active_contract.generation);
    assert(retire_begin_count == 0u);
    assert(clear_count == 0u);
    assert(pstvnc_h1_mpeg_presentation_owner_state(
        &coordinator.mpeg_handoff.owner) ==
        PSTVNC_H1_MPEG_PRESENTATION_MPEG_OWNED);

    /* An active worker blocks Pi retirement. */
    stop_poll_result = 0;
    enter_now = 0;
    assert(coordinator.interaction.calibration_entry_gate(
        coordinator.interaction.calibration_entry_gate_context,
        &enter_now));
    assert(!enter_now);
    assert(stop_poll_count >= 1u);
    assert(retire_begin_count == 0u);
    assert(clear_count == 0u);

    /* Worker completion is the authority to begin exact Pi retirement. */
    stop_poll_result = 1;
    enter_now = 0;
    assert(coordinator.interaction.calibration_entry_gate(
        coordinator.interaction.calibration_entry_gate_context,
        &enter_now));
    assert(!enter_now);
    assert(stop_complete_order != 0u);
    assert(retire_begin_count == 1u);
    assert(retire_generation == active_contract.generation);
    assert(coordinator.pi_retire_pending);
    assert(coordinator.pi_retire_generation == active_contract.generation);
    assert(clear_count == 0u);
    assert(stop_request_order < stop_complete_order);
    assert(stop_complete_order < retire_begin_order);

    /* Exact Pi ACK remains a fence before queue/presentation cleanup. */
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

    /*
     * ACK unlocks join/delete of the already-finished worker, graphics clear,
     * queue finalization, owner stop, and the required full-RFB restoration.
     */
    retire_poll_result = 1;
    enter_now = 0;
    assert(coordinator.interaction.calibration_entry_gate(
        coordinator.interaction.calibration_entry_gate_context,
        &enter_now));
    assert(!enter_now);
    assert(!coordinator.pi_retire_pending);
    assert(coordinator.pi_retire_generation == 0u);
    assert(!coordinator.mpeg_stop_pending);
    assert(coordinator.mpeg_stop_generation == 0u);
    assert(clear_count == 1u);
    assert(cleared_generation == active_contract.generation);
    assert(retire_finalize_count == 1u);
    assert(finalized_generation == active_contract.generation);
    assert(finalized_discarded == 17u);
    assert(retire_begin_order < clear_order);
    assert(clear_order < finalize_order);
    assert(transport_generation == 0u);
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

#!/usr/bin/env python3
"""One-shot repository patch for the CP2P session-coordinator refactor."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
H1 = ROOT / "experiments/media-harness-h1"
CAL = H1 / "mpeg_presentation_calibration"


def must_replace(text, old, new, label):
    if old not in text:
        raise SystemExit(f"missing patch anchor: {label}")
    return text.replace(old, new, 1)


# ---------------------------------------------------------------------------
# Interaction coordinator: explicit generic session seams.
# ---------------------------------------------------------------------------
header_path = H1 / "h1_interaction_coordinator.h"
header = header_path.read_text()
header = must_replace(
    header,
    "} pstvnc_h1_interaction_stats_t;\n\ntypedef struct pstvnc_h1_interaction_coordinator {",
    """} pstvnc_h1_interaction_stats_t;

/*
 * Optional session-level gate for a matured START+SELECT calibration request.
 * With no gate installed the legacy/CP2O behavior remains immediate entry.
 * A gate may defer entry while an owning session performs prerequisites such as
 * retiring MPEG and restoring one new full RFB frame.
 */
typedef int (*pstvnc_h1_interaction_calibration_entry_gate_fn)(
    void *context,
    int *enter_now);

typedef struct pstvnc_h1_interaction_coordinator {""",
    "interaction gate typedef",
)
header = must_replace(
    header,
    "    pstvnc_h1_mpeg_calibration_entry_hold_t mpeg_calibration_entry_hold;\n\n    const pstvnc_framebuffer_t *current_framebuffer;",
    """    pstvnc_h1_mpeg_calibration_entry_hold_t mpeg_calibration_entry_hold;

    pstvnc_h1_interaction_calibration_entry_gate_fn calibration_entry_gate;
    void *calibration_entry_gate_context;
    pstvnc_mpeg_cal_region_t accepted_calibration_region;
    unsigned calibration_entry_deferred : 1;
    unsigned accepted_calibration_pending : 1;

    const pstvnc_framebuffer_t *current_framebuffer;""",
    "interaction state fields",
)
header = must_replace(
    header,
    "void pstvnc_h1_interaction_coordinator_init(\n    pstvnc_h1_interaction_coordinator_t *coordinator);\n\n/* H1 RFB complete-frame presentation callback. */",
    """void pstvnc_h1_interaction_coordinator_init(
    pstvnc_h1_interaction_coordinator_t *coordinator);

/*
 * Install an optional session-level calibration-entry gate. Pass NULL to restore
 * immediate entry. This is configuration, not a transport callback; the
 * interaction coordinator remains ignorant of MPEG generations and PSTV START.
 */
int pstvnc_h1_interaction_coordinator_set_calibration_entry_gate(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_h1_interaction_calibration_entry_gate_fn gate,
    void *gate_context);

/*
 * Consume the one-shot accepted calibration result captured from the existing
 * calibration core. The copied region is the exact newly committed settings.
 */
int pstvnc_h1_interaction_coordinator_take_calibration_accept(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_mpeg_cal_region_t *committed_region);

/* H1 RFB complete-frame presentation callback. */""",
    "interaction public seams",
)
header_path.write_text(header)

source_path = H1 / "h1_interaction_coordinator.c"
source = source_path.read_text()
source = must_replace(
    source,
    "        !coordinator->mouse_interpretation_suspended &&\n        !pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(",
    "        !coordinator->mouse_interpretation_suspended &&\n        !coordinator->calibration_entry_deferred &&\n        !pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(",
    "deferred entry availability",
)

active_pattern = re.compile(
    r"static int h1_interaction_service_active_calibration\(.*?\n\}\n\nstatic int h1_interaction_service_controller_state",
    re.S,
)
active_replacement = r'''static int h1_interaction_record_calibration_result(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    const pstvnc_h1_mpeg_calibration_interaction_result_t *result)
{
    const pstvnc_mpeg_cal_region_t *committed_region;

    if (coordinator == NULL || result == NULL)
        return 0;

    if (!result->accepted)
        return 1;

    if (coordinator->accepted_calibration_pending)
        return 0;

    committed_region =
        pstvnc_h1_mpeg_calibration_interaction_binding_committed_region(
            &coordinator->mpeg_calibration);
    if (committed_region == NULL)
        return 0;

    coordinator->accepted_calibration_region = *committed_region;
    coordinator->accepted_calibration_pending = 1;
    return 1;
}

static int h1_interaction_service_active_calibration(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session,
    const pstvnc_controller_state_t *controller_state)
{
    pstvnc_h1_mpeg_calibration_interaction_context_t calibration_context;
    pstvnc_h1_mpeg_calibration_interaction_result_t result;

    h1_interaction_prepare_calibration_context(
        coordinator,
        session,
        &calibration_context);
    memset(&result, 0, sizeof(result));

    if (!pstvnc_h1_mpeg_calibration_interaction_binding_service_controller_result(
            &coordinator->mpeg_calibration,
            &calibration_context,
            controller_state,
            &result) ||
        !h1_interaction_record_calibration_result(coordinator, &result))
        return 0;

    /*
     * Once calibration owns foreground (including release quarantine), every
     * trustworthy controller observation belongs to it. Losing that consumption
     * fact would be an ownership violation, not a reason to fall through.
     */
    return result.consume_controller_state ? 1 : 0;
}

static int h1_interaction_service_controller_state'''
source, count = active_pattern.subn(active_replacement, source, count=1)
if count != 1:
    raise SystemExit("failed to replace active calibration service")

source = must_replace(
    source,
    """    if (pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
            &coordinator->mpeg_calibration))
        return h1_interaction_service_active_calibration(
            coordinator,
            session,
            controller_state);

    entry_available =""",
    """    if (pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
            &coordinator->mpeg_calibration))
        return h1_interaction_service_active_calibration(
            coordinator,
            session,
            controller_state);

    /*
     * A matured CP2P entry request owns controller routing while its session
     * prerequisite is pending. This keeps START/SELECT from leaking into OSK or
     * desktop actions during the one-full-RFB restoration interval.
     */
    if (coordinator->calibration_entry_deferred)
        return 1;

    entry_available =""",
    "deferred controller ownership",
)

hold_pattern = re.compile(
    r"static int h1_interaction_service_calibration_entry_hold\(.*?\n\}\n\nstatic int h1_interaction_resume_desktop_mouse_if_ready",
    re.S,
)
hold_replacement = r'''static int h1_interaction_query_calibration_entry_gate(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    int *enter_now)
{
    if (coordinator == NULL || enter_now == NULL)
        return 0;

    if (coordinator->calibration_entry_gate == NULL) {
        *enter_now = 1;
        return 1;
    }

    if (!coordinator->calibration_entry_gate(
            coordinator->calibration_entry_gate_context,
            enter_now))
        return 0;

    *enter_now = *enter_now ? 1 : 0;
    return 1;
}

static int h1_interaction_activate_calibration(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session)
{
    pstvnc_h1_mpeg_calibration_interaction_context_t calibration_context;
    pstvnc_h1_mpeg_calibration_interaction_result_t result;
    pstvnc_controller_state_t activation_state;

    if (coordinator == NULL || session == NULL)
        return 0;

    memset(&activation_state, 0, sizeof(activation_state));
    activation_state.buttons_down =
        PSTVNC_H1_MPEG_CALIBRATION_ENTRY_CHORD;
    memset(&result, 0, sizeof(result));

    h1_interaction_prepare_calibration_context(
        coordinator,
        session,
        &calibration_context);

    if (!pstvnc_h1_mpeg_calibration_interaction_binding_service_controller_result(
            &coordinator->mpeg_calibration,
            &calibration_context,
            &activation_state,
            &result) ||
        !h1_interaction_record_calibration_result(coordinator, &result))
        return 0;

    return
        result.consume_controller_state &&
        pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
            &coordinator->mpeg_calibration);
}

static int h1_interaction_service_calibration_entry_hold(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_rfb_session_t *session)
{
    int activate_calibration = 0;
    int enter_now = 0;

    if (coordinator == NULL || session == NULL)
        return 0;

    if (coordinator->calibration_entry_deferred) {
        if (!h1_interaction_query_calibration_entry_gate(
                coordinator, &enter_now))
            return 0;

        if (!enter_now)
            return 1;

        coordinator->calibration_entry_deferred = 0;
        return h1_interaction_activate_calibration(coordinator, session);
    }

    if (!pstvnc_h1_mpeg_calibration_entry_hold_poll(
            &coordinator->mpeg_calibration_entry_hold,
            h1_interaction_monotonic_us(),
            h1_interaction_calibration_entry_available(coordinator),
            &activate_calibration))
        return 0;

    if (!activate_calibration)
        return 1;

    if (!h1_interaction_query_calibration_entry_gate(
            coordinator, &enter_now))
        return 0;

    if (!enter_now) {
        coordinator->calibration_entry_deferred = 1;
        return 1;
    }

    return h1_interaction_activate_calibration(coordinator, session);
}

static int h1_interaction_resume_desktop_mouse_if_ready'''
source, count = hold_pattern.subn(hold_replacement, source, count=1)
if count != 1:
    raise SystemExit("failed to replace calibration entry service")

source = must_replace(
    source,
    """    pstvnc_h1_mpeg_calibration_entry_hold_init(
        &coordinator->mpeg_calibration_entry_hold);
}

int pstvnc_h1_interaction_coordinator_present""",
    """    pstvnc_h1_mpeg_calibration_entry_hold_init(
        &coordinator->mpeg_calibration_entry_hold);
}

int pstvnc_h1_interaction_coordinator_set_calibration_entry_gate(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_h1_interaction_calibration_entry_gate_fn gate,
    void *gate_context)
{
    if (coordinator == NULL || coordinator->input_started ||
        coordinator->calibration_entry_deferred ||
        pstvnc_h1_mpeg_calibration_interaction_binding_owns_foreground(
            &coordinator->mpeg_calibration))
        return 0;

    coordinator->calibration_entry_gate = gate;
    coordinator->calibration_entry_gate_context =
        gate != NULL ? gate_context : NULL;
    return 1;
}

int pstvnc_h1_interaction_coordinator_take_calibration_accept(
    pstvnc_h1_interaction_coordinator_t *coordinator,
    pstvnc_mpeg_cal_region_t *committed_region)
{
    if (coordinator == NULL || committed_region == NULL ||
        !coordinator->accepted_calibration_pending)
        return 0;

    *committed_region = coordinator->accepted_calibration_region;
    coordinator->accepted_calibration_pending = 0;
    return 1;
}

int pstvnc_h1_interaction_coordinator_present""",
    "interaction public implementation",
)
source_path.write_text(source)


# ---------------------------------------------------------------------------
# Shared compositor: generations may legitimately change source dimensions.
# ---------------------------------------------------------------------------
graphics_path = H1 / "h1_cumulative39_graphics.c"
graphics = graphics_path.read_text()
graphics_pattern = re.compile(
    r"static int h1_configure_video_texture\(.*?\n\}\n\nstatic void h1_detile_macroblocks",
    re.S,
)
graphics_replacement = r'''static int h1_configure_video_texture(
    unsigned int width,
    unsigned int height)
{
    if (!h1_video_texture_configured) {
        /*
         * Allocate one maximum-size VRAM backing store once. Each immutable MPEG
         * generation may then choose a different valid source size without
         * reallocating/leaking VRAM or inheriting dimensions from its predecessor.
         */
        memset(&h1_video_texture, 0, sizeof(h1_video_texture));
        h1_video_texture.PSM = GS_PSM_CT16;
        h1_video_texture.Mem = (u32 *)h1_video_linear;
        h1_video_texture.Filter = GS_FILTER_NEAREST;
        h1_video_texture.VramClut = 0;
        h1_video_texture.Vram = gsKit_vram_alloc(
            display,
            gsKit_texture_size(
                H1_VIDEO_MAX_WIDTH,
                H1_VIDEO_MAX_HEIGHT,
                GS_PSM_CT16),
            GSKIT_ALLOC_USERBUFFER);
        h1_video_texture_configured = 1;
    }

    h1_video_texture.Width = width;
    h1_video_texture.Height = height;
    h1_video_texture.Mem = (u32 *)h1_video_linear;
    h1_video_source_width = width;
    h1_video_source_height = height;
    return 0;
}

static void h1_detile_macroblocks'''
graphics, count = graphics_pattern.subn(graphics_replacement, graphics, count=1)
if count != 1:
    raise SystemExit("failed to replace video texture configuration")
graphics_path.write_text(graphics)


# ---------------------------------------------------------------------------
# Session coordinator: one owner for accepted -> prepare -> START and entry gate.
# ---------------------------------------------------------------------------
(H1 / "h1_cp2p_session_coordinator.h").write_text(r'''/*
 * File synopsis:
 * Defines CP2P's session-level owner for calibration acceptance, MPEG
 * generation start/retirement, the combined RFB flow policy, and the existing
 * PSTV transport/session identity.
 */
#ifndef PSTVNC_H1_CP2P_SESSION_COORDINATOR_H
#define PSTVNC_H1_CP2P_SESSION_COORDINATOR_H

#include "h1_interaction_coordinator.h"
#include "h1_transport_runtime.h"
#include "mpeg_presentation_calibration/h1_mpeg_cp2p_rfb_flow.h"
#include "mpeg_presentation_calibration/h1_mpeg_recalibration.h"
#include "mpeg_presentation_calibration/h1_mpeg_start_handoff.h"

typedef struct pstvnc_h1_cp2p_session_coordinator {
    pstvnc_h1_interaction_coordinator_t interaction;
    pstvnc_h1_mpeg_start_handoff_t mpeg_handoff;
    pstvnc_h1_mpeg_cp2p_rfb_flow_t rfb_flow;
    pstvnc_h1_rfb_flow_policy_t rfb_policy;
    pstvnc_h1_mpeg_recalibration_t recalibration;

    pstvnc_h1_transport_runtime_t *transport;
    pstvnc_h1_mpeg_recalibration_clear_mpeg_fn clear_mpeg;
    void *clear_mpeg_context;
    uint32_t session_id;

    pstvnc_h1_mpeg_start_contract_t current_start_contract;
    uint32_t start_messages_sent;
    unsigned current_start_contract_valid : 1;
    unsigned initialized : 1;
} pstvnc_h1_cp2p_session_coordinator_t;

int pstvnc_h1_cp2p_session_coordinator_init(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator,
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_mpeg_recalibration_clear_mpeg_fn clear_mpeg,
    void *clear_mpeg_context);

int pstvnc_h1_cp2p_session_coordinator_present(
    void *context,
    const pstvnc_framebuffer_t *framebuffer);

int pstvnc_h1_cp2p_session_coordinator_service(
    void *context,
    pstvnc_rfb_session_t *session);

const pstvnc_h1_rfb_flow_policy_t *
pstvnc_h1_cp2p_session_coordinator_rfb_policy(
    const pstvnc_h1_cp2p_session_coordinator_t *coordinator);

pstvnc_h1_mpeg_start_handoff_t *
pstvnc_h1_cp2p_session_coordinator_mpeg_handoff(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator);

const pstvnc_h1_mpeg_start_contract_t *
pstvnc_h1_cp2p_session_coordinator_current_start_contract(
    const pstvnc_h1_cp2p_session_coordinator_t *coordinator);

int pstvnc_h1_cp2p_session_coordinator_shutdown(
    pstvnc_h1_cp2p_session_coordinator_t *coordinator);

#endif /* PSTVNC_H1_CP2P_SESSION_COORDINATOR_H */
''')

(H1 / "h1_cp2p_session_coordinator.c").write_text(r'''/*
 * File synopsis:
 * Implements CP2P's one session-level lifecycle coordinator.
 *
 * A calibration accepted edge is consumed exactly once, converted into one
 * immutable generation by prepare_start(), and serialized as one START on the
 * existing PSTV socket using the transport-owned session id.
 */
#include "h1_cp2p_session_coordinator.h"

#include "mpeg_presentation_calibration/h1_mpeg_start_transport.h"

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
    pstvnc_h1_transport_runtime_t *transport,
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
''')


# ---------------------------------------------------------------------------
# Host contract for the new session owner.
# ---------------------------------------------------------------------------
(CAL / "h1_cp2p_session_coordinator_test.c").write_text(r'''/*
 * Host contract for the CP2P session coordinator.
 */
#include "../h1_cp2p_session_coordinator.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned int send_count;
static int send_success = 1;
static uint32_t sent_session_id;
static pstvnc_h1_mpeg_start_contract_t sent_contract;
static unsigned int clear_count;
static uint32_t cleared_generation;

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
    const pstvnc_h1_transport_runtime_t *runtime)
{
    if (runtime == NULL || !runtime->config_accepted)
        return NULL;
    return &runtime->config;
}

int pstvnc_h1_mpeg_start_transport_send(
    pstvnc_h1_transport_runtime_t *runtime,
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
    pstvnc_h1_transport_runtime_t transport;
    pstvnc_rfb_session_t rfb_session;
    pstvnc_mpeg_cal_region_t region;
    pstvnc_h1_mpeg_start_contract_t active_contract;
    pstvnc_h1_rfb_request_policy_decision_t decision;
    int enter_now = 0;

    memset(&transport, 0, sizeof(transport));
    memset(&rfb_session, 0, sizeof(rfb_session));
    transport.config_accepted = 1;
    transport.config.session_id = UINT32_C(0x1234abcd);

    assert(pstvnc_h1_cp2p_session_coordinator_init(
        &coordinator, &transport, clear_mpeg, NULL));

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

    send_success = 1;
    region = sample_region(128, 80);
    assert(pstvnc_h1_mpeg_start_handoff_prepare_start(
        &coordinator.mpeg_handoff, &region, &active_contract));
    assert(pstvnc_h1_mpeg_start_handoff_first_frame_presented(
        &coordinator.mpeg_handoff, active_contract.generation));
    coordinator.current_start_contract = active_contract;
    coordinator.current_start_contract_valid = 1;

    assert(coordinator.interaction.calibration_entry_gate(
        coordinator.interaction.calibration_entry_gate_context,
        &enter_now));
    assert(!enter_now);
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
''')


# ---------------------------------------------------------------------------
# Host makefile: replace the old global bridge contract with session contract.
# ---------------------------------------------------------------------------
host_make_path = CAL / "Makefile.host"
host_make = host_make_path.read_text()
host_make, count = re.subn(
    r"\nRECALIBRATION_ENTRY_BRIDGE_SOURCES = \\\n(?:.*\n){3}",
    "\n",
    host_make,
    count=1,
)
if count != 1:
    raise SystemExit("failed to remove bridge source group")
host_make = must_replace(
    host_make,
    "RECALIBRATION_TEST = h1_mpeg_recalibration_test\nRECALIBRATION_ENTRY_BRIDGE_TEST = h1_mpeg_recalibration_entry_bridge_test",
    "RECALIBRATION_TEST = h1_mpeg_recalibration_test\nCP2P_SESSION_COORDINATOR_TEST = h1_cp2p_session_coordinator_test",
    "host test variable",
)
host_make = host_make.replace(
    " $(RECALIBRATION_TEST) $(RECALIBRATION_ENTRY_BRIDGE_TEST)",
    " $(RECALIBRATION_TEST) $(CP2P_SESSION_COORDINATOR_TEST)",
)
host_make = must_replace(
    host_make,
    "\t./$(RECALIBRATION_TEST)\n\t./$(RECALIBRATION_ENTRY_BRIDGE_TEST)",
    "\t./$(RECALIBRATION_TEST)\n\t./$(CP2P_SESSION_COORDINATOR_TEST)",
    "host test execution",
)
bridge_rule = re.compile(
    r"\n\$\(RECALIBRATION_ENTRY_BRIDGE_TEST\):.*?\n\t\$\(CC\).*? -o \$@\n",
    re.S,
)
session_rule = """
$(CP2P_SESSION_COORDINATOR_TEST): $(RECALIBRATION_SOURCES) ../h1_cp2p_session_coordinator.c h1_cp2p_session_coordinator_test.c
	$(CC) $(CFLAGS) -I. -I.. -Ihost_stubs -I../../audio-transport/common -I../../../src/input -I../../../src/framebuffer -I../../../src/rfb $(RECALIBRATION_SOURCES) ../h1_cp2p_session_coordinator.c h1_cp2p_session_coordinator_test.c -o $@
"""
host_make, count = bridge_rule.subn(session_rule, host_make, count=1)
if count != 1:
    raise SystemExit("failed to replace bridge test rule")
host_make = host_make.replace(
    " $(RECALIBRATION_TEST) $(RECALIBRATION_ENTRY_BRIDGE_TEST)\n",
    " $(RECALIBRATION_TEST) $(CP2P_SESSION_COORDINATOR_TEST)\n",
)
host_make_path.write_text(host_make)


# ---------------------------------------------------------------------------
# Pinned PS2 compile diagnostic follows the current architecture only.
# ---------------------------------------------------------------------------
(ROOT / "mk/media-harness-h1-cp2p-compositor-check.mk").write_text(r'''# CP2P calibrated compositor/runtime/session-coordinator compile diagnostic.
#
# This target is compile-only. It proves that the pinned PS2 EE/gsKit toolchain
# accepts the shared graphics owner, generated MPEG runtime, ordinary interaction
# coordinator, current recalibration helper, and session-level coordinator.

BUILD_DIR := build/experiments/media-harness-h1-cp2p-compositor-check/ps2
CP2P_GEN_DIR := $(BUILD_DIR)/generated-cp2p
CP2P_VIDEO_GENERATED := $(CP2P_GEN_DIR)/h1_video_runtime_cp2p_generated.c
CALIBRATION_DIR := experiments/media-harness-h1/mpeg_presentation_calibration

include mk/media-harness-h1-cumulative39-thread-census.mk

.PHONY: cp2p-compositor-check

cp2p-compositor-check: \
	$(BUILD_DIR)/h1_cp2p_shared_compositor_check.o \
	$(BUILD_DIR)/h1_cp2p_video_runtime_check.o \
	$(BUILD_DIR)/h1_cp2p_interaction_coordinator_check.o \
	$(BUILD_DIR)/h1_cp2p_recalibration_check.o \
	$(BUILD_DIR)/h1_cp2p_session_coordinator_check.o
	@echo H1_CP2P_SHARED_COMPOSITOR_COMPILE=PASS
	@echo H1_CP2P_VIDEO_RUNTIME_COMPILE=PASS
	@echo H1_CP2P_INTERACTION_COORDINATOR_COMPILE=PASS
	@echo H1_CP2P_RECALIBRATION_COMPILE=PASS
	@echo H1_CP2P_SESSION_COORDINATOR_COMPILE=PASS

$(CP2P_GEN_DIR):
	mkdir -p $@

$(BUILD_DIR)/h1_cp2p_shared_compositor_check.o: \
	experiments/media-harness-h1/h1_cumulative39_graphics.c \
	experiments/media-harness-h1/h1_cumulative39_graphics.h \
	$(CALIBRATION_DIR)/h1_mpeg_start_handoff.h \
	$(CALIBRATION_DIR)/h1_mpeg_presentation_owner.h \
	$(CALIBRATION_DIR)/mpeg_presentation_calibration.h \
	src/platform/ps2_graphics.c \
	src/platform/ps2_graphics.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(CP2P_VIDEO_GENERATED): \
	experiments/media-harness-h1/generate_h1_video_runtime_cp2p.py \
	experiments/media-harness-h1/h1_video_runtime.c | $(CP2P_GEN_DIR)
	python3 experiments/media-harness-h1/generate_h1_video_runtime_cp2p.py \
		--input experiments/media-harness-h1/h1_video_runtime.c \
		--output $@

$(BUILD_DIR)/h1_cp2p_video_runtime_check.o: \
	$(CP2P_VIDEO_GENERATED) \
	experiments/media-harness-h1/h1_video_runtime.h \
	experiments/media-harness-h1/h1_video_runtime_cp2p.h \
	experiments/media-harness-h1/h1_cumulative39_graphics.h \
	$(CALIBRATION_DIR)/h1_mpeg_start_handoff.h \
	$(CALIBRATION_DIR)/h1_mpeg_presentation_owner.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $(CP2P_VIDEO_GENERATED) -o $@

$(BUILD_DIR)/h1_cp2p_interaction_coordinator_check.o: \
	experiments/media-harness-h1/h1_interaction_coordinator.c \
	experiments/media-harness-h1/h1_interaction_coordinator.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $< -o $@

$(BUILD_DIR)/h1_cp2p_recalibration_check.o: \
	$(CALIBRATION_DIR)/h1_mpeg_recalibration.c \
	$(CALIBRATION_DIR)/h1_mpeg_recalibration.h \
	$(CALIBRATION_DIR)/h1_mpeg_cp2p_rfb_flow.h \
	$(CALIBRATION_DIR)/h1_mpeg_start_handoff.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $< -o $@

$(BUILD_DIR)/h1_cp2p_session_coordinator_check.o: \
	experiments/media-harness-h1/h1_cp2p_session_coordinator.c \
	experiments/media-harness-h1/h1_cp2p_session_coordinator.h \
	experiments/media-harness-h1/h1_interaction_coordinator.h \
	experiments/media-harness-h1/h1_transport_runtime.h \
	$(CALIBRATION_DIR)/h1_mpeg_recalibration.h \
	$(CALIBRATION_DIR)/h1_mpeg_start_transport.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $< -o $@
''')


# Remove the proof-only path now superseded by the explicit session owner.
for relative in [
    "h1_interaction_coordinator_cp2p.c",
    "h1_interaction_cp2p.c",
    "h1_interaction_cp2p.h",
    "mpeg_presentation_calibration/h1_mpeg_recalibration_entry_bridge.c",
    "mpeg_presentation_calibration/h1_mpeg_recalibration_entry_bridge.h",
    "mpeg_presentation_calibration/h1_mpeg_recalibration_entry_bridge_test.c",
]:
    path = H1 / relative
    if path.exists():
        path.unlink()

# Append only; older proof-bridge entries remain historical truth.
history = H1 / "CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md"
with history.open("a") as handle:
    handle.write(r'''

---

## PHASE K — SESSION COORDINATOR REPLACES THE PROOF BRIDGE

### 33. September 12 — accepted calibration becomes a session-owned MPEG START transaction

The next implementation pass replaced the temporary CP2P interaction/macro bridge with the session-level ownership boundary agreed during live review.

At this point the working lifecycle was: interaction reports the one-shot accepted committed settings; the session coordinator owns transport/session identity; the MPEG handoff creates one immutable generation and enters `WAIT_FIRST_FRAME`; exactly that generation is sent once as START on the existing PSTV socket/channel 4. START+SELECT does not edit a generation in place: if one exists, its confirmed geometry/mattes seed the next calibration transaction, it is retired, one new full RFB restoration is required, and only then does calibration enter. Cancel leaves no MPEG generation; confirm creates a fresh generation.

The prior `h1_mpeg_recalibration_entry_bridge` global interception and CP2P coordinator-by-macro-include/wrapper files were deliberately removed as proof scaffolding. Their earlier commits remain valid historical evidence; they are not retained as competing runtime paths.

The ordinary interaction coordinator instead gained two explicit generic seams: an optional calibration-entry gate (unset preserves immediate-entry behavior) and a one-shot accepted-calibration accessor. CP2P installs the gate from its session coordinator; CP2O does not.

The shared compositor's earlier first-qualification restriction against a later MPEG source size was also retired. It now allocates one maximum-size video VRAM backing store once and changes only texture dimensions per immutable generation, avoiding both dimension inheritance and repeated VRAM allocation.

This entry records source/host/toolchain evidence only. It does not claim a runnable all-guns ELF or hardware qualification.
''')

# The patch mechanism is deliberately one-shot and must not survive its commit.
for relative in [
    ".github/workflows/cp2p-session-coordinator-apply.yml",
    "experiments/media-harness-h1/apply_cp2p_session_refactor.py",
]:
    path = ROOT / relative
    if path.exists():
        path.unlink()

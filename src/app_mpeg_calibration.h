/*
 * File synopsis:
 * Defines the Application-owned manual MPEG CALIBRATION foreground transaction
 * reconstructed by A004 P9 and its narrow P10 protected-start handoff seams.
 *
 * This coordinator composes already-owned P2 RFB protection, input-runtime
 * mouse suspension/rebase, published-pointer neutralization, the accepted P8
 * manual MPEG CALIBRATION region source, Platform desktop presentation, and P3
 * read-only presentation state.
 *
 * It deliberately does not choose a calibration entry gesture, route ordinary
 * local-controller/OSK actions, mutate P3, start/retire/service MPEG, own RFB
 * parsing/framebuffer truth, poll libpad, persist configuration, or implement
 * DESKTOP CALIBRATION.
 *
 * Context:
 *   docs/ledge/LEDGE_FOREMAN_STATE.md,
 *   A004-APPLICATION-MPEG-CALIBRATION-FOREGROUND-P9;
 *   docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md.
 */

#ifndef PSTVNC_APP_MPEG_CALIBRATION_H
#define PSTVNC_APP_MPEG_CALIBRATION_H

#include <stddef.h>
#include <stdint.h>

#include "display/mpeg_presentation.h"
#include "input/input_runtime.h"
#include "rfb/flow_policy.h"
#include "rfb/rfb_session.h"
#include "ui/local_ui.h"
#include "ui/mpeg_calibration_manual.h"

typedef enum pstvnc_app_mpeg_calibration_state {
    PSTVNC_APP_MPEG_CALIBRATION_IDLE = 0,
    PSTVNC_APP_MPEG_CALIBRATION_ACTIVE,
    PSTVNC_APP_MPEG_CALIBRATION_ACCEPTED_PROTECTED,
    PSTVNC_APP_MPEG_CALIBRATION_FAULTED
} pstvnc_app_mpeg_calibration_state_t;

typedef enum pstvnc_app_mpeg_calibration_result {
    PSTVNC_APP_MPEG_CALIBRATION_OK = 0,
    PSTVNC_APP_MPEG_CALIBRATION_INVALID = -1,
    PSTVNC_APP_MPEG_CALIBRATION_ADMISSION_REJECTED = -2,
    PSTVNC_APP_MPEG_CALIBRATION_FREEZE_FAILED = -3,
    PSTVNC_APP_MPEG_CALIBRATION_INPUT_SUSPEND_FAILED = -4,
    PSTVNC_APP_MPEG_CALIBRATION_POINTER_RELEASE_FAILED = -5,
    PSTVNC_APP_MPEG_CALIBRATION_INPUT_REBASE_FAILED = -6,
    PSTVNC_APP_MPEG_CALIBRATION_SNAPSHOT_FAILED = -7,
    PSTVNC_APP_MPEG_CALIBRATION_MANUAL_BEGIN_FAILED = -8,
    PSTVNC_APP_MPEG_CALIBRATION_MANUAL_SERVICE_FAILED = -9,
    PSTVNC_APP_MPEG_CALIBRATION_GEOMETRY_FAILED = -10,
    PSTVNC_APP_MPEG_CALIBRATION_RASTER_FAILED = -11,
    PSTVNC_APP_MPEG_CALIBRATION_PRESENT_FAILED = -12,
    PSTVNC_APP_MPEG_CALIBRATION_STATE_CONTRADICTION = -13,
    PSTVNC_APP_MPEG_CALIBRATION_INPUT_RESUME_FAILED = -14,
    PSTVNC_APP_MPEG_CALIBRATION_THAW_FAILED = -15,
    PSTVNC_APP_MPEG_CALIBRATION_NOT_ACTIVE = -16,
    PSTVNC_APP_MPEG_CALIBRATION_NOT_ACCEPTED_PROTECTED = -17,
    PSTVNC_APP_MPEG_CALIBRATION_ALREADY_FAULTED = -18,
    PSTVNC_APP_MPEG_CALIBRATION_HANDOFF_STATE_INVALID = -19,
    PSTVNC_APP_MPEG_CALIBRATION_HANDOFF_FAULTED = -20
} pstvnc_app_mpeg_calibration_result_t;

typedef struct pstvnc_app_mpeg_calibration_service_result {
    unsigned consume_controller_state : 1;
    unsigned accepted_edge : 1;
    unsigned cancelled_edge : 1;
    unsigned foreground_completed : 1;
    unsigned accepted_protected : 1;
    unsigned cancelled_completed : 1;
} pstvnc_app_mpeg_calibration_service_result_t;

typedef struct pstvnc_app_mpeg_calibration {
    pstvnc_mpeg_calibration_manual_source_t manual_source;

    pstvnc_rfb_flow_policy_t *rfb_flow_policy;
    pstvnc_input_runtime_t *input_runtime;
    pstvnc_rfb_session_t *rfb_session;
    const pstvnc_local_ui_t *local_ui;
    pstvnc_mpeg_presentation_t *presentation;

    uint16_t *frozen_desktop;
    uint16_t *work_surface;
    size_t surface_capacity_pixels;
    size_t canvas_pixel_count;
    int32_t canvas_width;
    int32_t canvas_height;

    pstvnc_mpeg_presentation_geometry_t accepted_geometry;
    pstvnc_app_mpeg_calibration_state_t state;
    pstvnc_app_mpeg_calibration_result_t last_result;

    unsigned mouse_interpretation_suspended : 1;
    unsigned accepted_pending_release : 1;
    unsigned cancelled_pending_release : 1;
    unsigned accepted_geometry_valid : 1;
} pstvnc_app_mpeg_calibration_t;

/*
 * Initialize one reusable Application foreground coordinator around two
 * caller-owned CT16 surfaces.
 *
 * frozen_desktop receives an immutable copy of the last physically presented
 * ordinary desktop at begin. work_surface is separate disposable calibration
 * presentation data. Neither surface becomes authoritative RFB framebuffer
 * storage.
 */
int pstvnc_app_mpeg_calibration_init(
    pstvnc_app_mpeg_calibration_t *calibration,
    int32_t canvas_width,
    int32_t canvas_height,
    uint16_t *frozen_desktop,
    uint16_t *work_surface,
    size_t surface_capacity_pixels);

/*
 * Begin one trigger-agnostic RFB-only manual calibration transaction.
 *
 * Admission requires ordinary DESKTOP local UI with no quarantine, exact P3
 * RFB_ONLY/no snapshot state, thawed P2, inactive P8, and idle coordinator.
 *
 * After admission the exact order is:
 *   P2 freeze -> input mouse suspend -> optional remote button-up publication
 *   -> suspended mouse rebase -> frozen desktop snapshot -> P8 begin
 *   -> first local calibration presentation.
 *
 * published_click_buttons is Application-owned successfully-published semantic
 * click authority. It becomes zero only after an actually successful RFB
 * PointerEvent release.
 */
pstvnc_app_mpeg_calibration_result_t
pstvnc_app_mpeg_calibration_begin(
    pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_rfb_flow_policy_t *rfb_flow_policy,
    pstvnc_input_runtime_t *input_runtime,
    pstvnc_rfb_session_t *rfb_session,
    const pstvnc_local_ui_t *local_ui,
    pstvnc_mpeg_presentation_t *presentation,
    unsigned int published_cursor_x,
    unsigned int published_cursor_y,
    unsigned char *published_click_buttons,
    const uint16_t *last_presented_desktop,
    size_t last_presented_pixel_count);

/*
 * Service one normalized physical-controller observation while P8 owns the
 * manual foreground.
 *
 * Visible EDIT/REVIEW states are rastered from the immutable frozen snapshot
 * into work_surface and physically presented through the ordinary Platform
 * desktop seam. Accept/cancel do not leave foreground until P8's release
 * quarantine completes.
 */
pstvnc_app_mpeg_calibration_result_t
pstvnc_app_mpeg_calibration_service_controller(
    pstvnc_app_mpeg_calibration_t *calibration,
    const pstvnc_controller_state_t *controller_state,
    pstvnc_app_mpeg_calibration_service_result_t *result);

/*
 * Copy the exact P1-resolved presentation geometry retained by a completed
 * protected acceptance. This read has no P2 or foreground side effect.
 */
int pstvnc_app_mpeg_calibration_copy_accepted_geometry(
    const pstvnc_app_mpeg_calibration_t *calibration,
    pstvnc_mpeg_presentation_geometry_t *geometry);

/*
 * Abandon one ACCEPTED_PROTECTED endpoint without starting MPEG.
 *
 * The operation thaws P2 exactly once, thereby preserving P2's normal one-shot
 * FULL-refresh debt, clears the retained protected geometry, and returns idle.
 */
pstvnc_app_mpeg_calibration_result_t
pstvnc_app_mpeg_calibration_abort_accepted(
    pstvnc_app_mpeg_calibration_t *calibration);

/*
 * Commit one already-proven protected transfer into an exact P3
 * WAIT_FIRST_FRAME run without thawing P2.
 *
 * The retained accepted geometry must match P3's immutable snapshot for the
 * supplied nonzero generation. Success clears only P9's protected-calibration
 * authority and returns P9 to reusable IDLE. P2, P3 and the run owner are not
 * mutated by this operation.
 */
pstvnc_app_mpeg_calibration_result_t
pstvnc_app_mpeg_calibration_commit_protected_handoff(
    pstvnc_app_mpeg_calibration_t *calibration,
    uint32_t run_generation);

/*
 * Fault-contain one ACCEPTED_PROTECTED handoff after downstream run authority
 * may have been acquired. This never thaws P2 and deliberately retains the
 * geometry/owner evidence for outer teardown diagnosis.
 */
pstvnc_app_mpeg_calibration_result_t
pstvnc_app_mpeg_calibration_fault_protected_handoff(
    pstvnc_app_mpeg_calibration_t *calibration);

pstvnc_app_mpeg_calibration_state_t
pstvnc_app_mpeg_calibration_state(
    const pstvnc_app_mpeg_calibration_t *calibration);

#endif /* PSTVNC_APP_MPEG_CALIBRATION_H */

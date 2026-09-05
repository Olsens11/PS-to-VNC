/*
 * File synopsis:
 * Defines the minimal PS2-local foreground/UI ownership model used by
 * Reconstruction Stage 2.
 *
 * This module owns only local UI state:
 *
 *   - whether the remote desktop or OSK owns foreground interaction;
 *   - the foreground return relationship;
 *   - quarantine across a foreground ownership transition;
 *   - local surface generation/dirty publication state.
 *
 * It does not poll libpad, interpret physical controller buttons, serialize
 * RFB, draw pixels, present GS frames, or execute product actions.
 *
 * Context:
 *   docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md, B09;
 *   docs/CLEAN_ARCHITECTURE.md, Local UI ownership;
 *   GitHub Issue #39.
 */

#ifndef PSTVNC_LOCAL_UI_H
#define PSTVNC_LOCAL_UI_H

#include <stdint.h>

/*
 * Stage 2 initially needs only two foreground owners.
 *
 * DESKTOP means ordinary remote-desktop interaction owns semantic input.
 * OSK means PS2-local on-screen-keyboard interaction owns semantic input.
 *
 * Later menu families should extend this only when they are actually earned.
 */
typedef enum pstvnc_local_ui_foreground {
    PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP = 0,
    PSTVNC_LOCAL_UI_FOREGROUND_OSK
} pstvnc_local_ui_foreground_t;

/*
 * Complete main-owned local UI state.
 *
 * return_foreground records where the current local foreground returns when it
 * closes. For the first Stage 2 OSK slice that relationship is DESKTOP->OSK->
 * DESKTOP, but storing it explicitly prevents close behavior from depending on
 * an implicit global assumption.
 *
 * input_quarantined is asserted whenever foreground ownership changes. The UI
 * itself does not decide when controller state is physically safe again. The
 * application/input boundary must explicitly complete quarantine after proving
 * that the transition-owning gesture cannot leak into the new context.
 *
 * generation advances for every visible local-state mutation. dirty is the
 * presentation authority; presented_generation records which exact generation
 * was last acknowledged as locally presented.
 */
typedef struct pstvnc_local_ui {
    pstvnc_local_ui_foreground_t foreground;
    pstvnc_local_ui_foreground_t return_foreground;

    int input_quarantined;

    uint32_t generation;
    uint32_t presented_generation;
    int dirty;
} pstvnc_local_ui_t;

/*
 * Initialize a clean ordinary-desktop state.
 *
 * No local overlay is visible, no transition quarantine is active, and there
 * is no local-only presentation work pending.
 */
void pstvnc_local_ui_init(
    pstvnc_local_ui_t *ui);

/*
 * Enter OSK foreground ownership.
 *
 * The transition is accepted only from DESKTOP. It records DESKTOP as the
 * return foreground, asserts input quarantine, and marks the local surface
 * dirty at a new generation.
 *
 * Returns 1 when the transition occurs, otherwise 0 without mutation.
 */
int pstvnc_local_ui_open_osk(
    pstvnc_local_ui_t *ui);

/*
 * Close OSK foreground ownership and return to its recorded underlay owner.
 *
 * The transition asserts a fresh quarantine so the close gesture cannot become
 * an action in the returned desktop context. The disappearing overlay is also
 * a local visual change and therefore advances generation/dirty state.
 *
 * Returns 1 when the transition occurs, otherwise 0 without mutation.
 */
int pstvnc_local_ui_close_osk(
    pstvnc_local_ui_t *ui);

/*
 * Report whether semantic controller actions must currently be quarantined.
 */
int pstvnc_local_ui_input_is_quarantined(
    const pstvnc_local_ui_t *ui);

/*
 * Complete the current input-quarantine boundary.
 *
 * Application policy may call this only after the upstream input/controller
 * responsibility has proven the transition-owning gesture cannot leak into the
 * current foreground context.
 *
 * This function deliberately knows no libpad or physical-button vocabulary.
 *
 * Returns 1 when an active quarantine is cleared, otherwise 0.
 */
int pstvnc_local_ui_complete_input_quarantine(
    pstvnc_local_ui_t *ui);

/*
 * Record a visible PS2-local UI mutation such as future OSK selection or
 * modifier-state changes.
 *
 * This does not draw or present anything; it only advances local presentation
 * authority.
 */
int pstvnc_local_ui_mark_local_change(
    pstvnc_local_ui_t *ui);

/*
 * Return the exact current local generation.
 */
uint32_t pstvnc_local_ui_generation(
    const pstvnc_local_ui_t *ui);

/*
 * Report whether local presentation work is outstanding.
 */
int pstvnc_local_ui_needs_present(
    const pstvnc_local_ui_t *ui);

/*
 * Acknowledge that one exact local generation was successfully presented.
 *
 * A stale acknowledgement is rejected and leaves dirty asserted. This keeps a
 * newer local mutation from being accidentally declared visible merely because
 * an older render/present operation completed.
 *
 * Returns 1 only when generation matches the current local generation.
 */
int pstvnc_local_ui_mark_presented(
    pstvnc_local_ui_t *ui,
    uint32_t generation);

#endif /* PSTVNC_LOCAL_UI_H */

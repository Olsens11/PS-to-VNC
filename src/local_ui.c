/*
 * File synopsis:
 * Implements pure Stage 2 local foreground ownership, context-transition
 * quarantine, and local generation/dirty publication state.
 *
 * No controller, RFB, framebuffer, GS, OSK-layout, or product-action mechanism
 * belongs in this translation unit.
 *
 * Context:
 *   docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md, B09;
 *   GitHub Issue #39.
 */

#include <stddef.h>
#include <stdint.h>

#include "local_ui.h"

static void local_ui_advance_generation(
    pstvnc_local_ui_t *ui)
{
    /*
     * dirty is the authoritative outstanding-presentation fact. Generation is
     * the identity used to reject a stale presentation acknowledgement.
     *
     * Skip zero on wrap so zero remains the recognizable initialized baseline.
     */
    ui->generation++;

    if (ui->generation == 0)
        ui->generation = 1;

    ui->dirty = 1;
}

void pstvnc_local_ui_init(
    pstvnc_local_ui_t *ui)
{
    if (ui == NULL)
        return;

    ui->foreground =
        PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP;

    ui->return_foreground =
        PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP;

    ui->input_quarantined = 0;

    ui->generation = 0;
    ui->presented_generation = 0;
    ui->dirty = 0;
}

int pstvnc_local_ui_open_osk(
    pstvnc_local_ui_t *ui)
{
    if (ui == NULL ||
        ui->foreground !=
            PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP)
        return 0;

    ui->return_foreground = ui->foreground;
    ui->foreground =
        PSTVNC_LOCAL_UI_FOREGROUND_OSK;

    /*
     * No controller gesture from the old desktop ownership epoch may become a
     * newly interpreted OSK action after this boundary.
     */
    ui->input_quarantined = 1;

    local_ui_advance_generation(ui);
    return 1;
}

int pstvnc_local_ui_close_osk(
    pstvnc_local_ui_t *ui)
{
    pstvnc_local_ui_foreground_t destination;

    if (ui == NULL ||
        ui->foreground !=
            PSTVNC_LOCAL_UI_FOREGROUND_OSK)
        return 0;

    destination = ui->return_foreground;

    /*
     * The current Stage 2 contract permits OSK only above DESKTOP. Reject
     * corrupted/speculative return ownership instead of widening the state
     * machine before another foreground family is actually earned.
     */
    if (destination !=
        PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP)
        return 0;

    ui->foreground = destination;
    ui->return_foreground =
        PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP;

    /*
     * Quarantine the close gesture as it crosses back into desktop ownership.
     */
    ui->input_quarantined = 1;

    local_ui_advance_generation(ui);
    return 1;
}

int pstvnc_local_ui_input_is_quarantined(
    const pstvnc_local_ui_t *ui)
{
    if (ui == NULL)
        return 0;

    return ui->input_quarantined != 0;
}

int pstvnc_local_ui_complete_input_quarantine(
    pstvnc_local_ui_t *ui)
{
    if (ui == NULL ||
        !ui->input_quarantined)
        return 0;

    ui->input_quarantined = 0;
    return 1;
}

int pstvnc_local_ui_mark_local_change(
    pstvnc_local_ui_t *ui)
{
    if (ui == NULL)
        return 0;

    local_ui_advance_generation(ui);
    return 1;
}

uint32_t pstvnc_local_ui_generation(
    const pstvnc_local_ui_t *ui)
{
    if (ui == NULL)
        return 0;

    return ui->generation;
}

int pstvnc_local_ui_needs_present(
    const pstvnc_local_ui_t *ui)
{
    if (ui == NULL)
        return 0;

    return ui->dirty != 0;
}

int pstvnc_local_ui_mark_presented(
    pstvnc_local_ui_t *ui,
    uint32_t generation)
{
    if (ui == NULL ||
        !ui->dirty ||
        generation != ui->generation)
        return 0;

    ui->presented_generation = generation;
    ui->dirty = 0;

    return 1;
}

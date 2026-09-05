#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "local_ui.h"

static void test_initial_state_is_clean_desktop(void)
{
    pstvnc_local_ui_t ui;

    pstvnc_local_ui_init(&ui);

    assert(
        ui.foreground ==
        PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP);

    assert(
        ui.return_foreground ==
        PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP);

    assert(
        !pstvnc_local_ui_input_is_quarantined(
            &ui));

    assert(
        pstvnc_local_ui_generation(&ui) ==
        0u);

    assert(
        !pstvnc_local_ui_needs_present(
            &ui));

    assert(ui.presented_generation == 0u);
}

static void test_open_osk_changes_foreground_and_quarantines(void)
{
    pstvnc_local_ui_t ui;

    pstvnc_local_ui_init(&ui);

    assert(
        pstvnc_local_ui_open_osk(
            &ui));

    assert(
        ui.foreground ==
        PSTVNC_LOCAL_UI_FOREGROUND_OSK);

    assert(
        ui.return_foreground ==
        PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP);

    assert(
        pstvnc_local_ui_input_is_quarantined(
            &ui));

    assert(
        pstvnc_local_ui_generation(&ui) ==
        1u);

    assert(
        pstvnc_local_ui_needs_present(
            &ui));
}

static void test_repeated_open_is_rejected_without_mutation(void)
{
    pstvnc_local_ui_t ui;
    uint32_t generation;

    pstvnc_local_ui_init(&ui);

    assert(
        pstvnc_local_ui_open_osk(
            &ui));

    generation =
        pstvnc_local_ui_generation(&ui);

    assert(
        !pstvnc_local_ui_open_osk(
            &ui));

    assert(
        pstvnc_local_ui_generation(&ui) ==
        generation);

    assert(
        ui.foreground ==
        PSTVNC_LOCAL_UI_FOREGROUND_OSK);

    assert(
        pstvnc_local_ui_input_is_quarantined(
            &ui));
}

static void test_quarantine_requires_explicit_completion(void)
{
    pstvnc_local_ui_t ui;

    pstvnc_local_ui_init(&ui);

    assert(
        !pstvnc_local_ui_complete_input_quarantine(
            &ui));

    assert(
        pstvnc_local_ui_open_osk(
            &ui));

    assert(
        pstvnc_local_ui_input_is_quarantined(
            &ui));

    assert(
        pstvnc_local_ui_complete_input_quarantine(
            &ui));

    assert(
        !pstvnc_local_ui_input_is_quarantined(
            &ui));

    assert(
        !pstvnc_local_ui_complete_input_quarantine(
            &ui));
}

static void test_local_change_advances_dirty_generation(void)
{
    pstvnc_local_ui_t ui;

    pstvnc_local_ui_init(&ui);

    assert(
        pstvnc_local_ui_open_osk(
            &ui));

    assert(
        pstvnc_local_ui_generation(&ui) ==
        1u);

    assert(
        pstvnc_local_ui_mark_local_change(
            &ui));

    assert(
        pstvnc_local_ui_generation(&ui) ==
        2u);

    assert(
        pstvnc_local_ui_needs_present(
            &ui));
}

static void test_stale_present_ack_cannot_clear_newer_change(void)
{
    pstvnc_local_ui_t ui;
    uint32_t first_generation;
    uint32_t second_generation;

    pstvnc_local_ui_init(&ui);

    assert(
        pstvnc_local_ui_open_osk(
            &ui));

    first_generation =
        pstvnc_local_ui_generation(&ui);

    assert(
        pstvnc_local_ui_mark_local_change(
            &ui));

    second_generation =
        pstvnc_local_ui_generation(&ui);

    assert(second_generation != first_generation);

    /*
     * An older render/present completion cannot claim the newer local state.
     */
    assert(
        !pstvnc_local_ui_mark_presented(
            &ui,
            first_generation));

    assert(
        pstvnc_local_ui_needs_present(
            &ui));

    assert(
        pstvnc_local_ui_mark_presented(
            &ui,
            second_generation));

    assert(
        !pstvnc_local_ui_needs_present(
            &ui));

    assert(
        ui.presented_generation ==
        second_generation);
}

static void test_close_returns_to_desktop_with_fresh_quarantine(void)
{
    pstvnc_local_ui_t ui;
    uint32_t open_generation;

    pstvnc_local_ui_init(&ui);

    assert(
        pstvnc_local_ui_open_osk(
            &ui));

    assert(
        pstvnc_local_ui_complete_input_quarantine(
            &ui));

    open_generation =
        pstvnc_local_ui_generation(&ui);

    assert(
        pstvnc_local_ui_mark_presented(
            &ui,
            open_generation));

    assert(
        !pstvnc_local_ui_needs_present(
            &ui));

    assert(
        pstvnc_local_ui_close_osk(
            &ui));

    assert(
        ui.foreground ==
        PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP);

    assert(
        ui.return_foreground ==
        PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP);

    assert(
        pstvnc_local_ui_input_is_quarantined(
            &ui));

    assert(
        pstvnc_local_ui_generation(&ui) !=
        open_generation);

    /*
     * Removing the overlay itself requires a fresh local presentation.
     */
    assert(
        pstvnc_local_ui_needs_present(
            &ui));
}

static void test_close_outside_osk_is_rejected(void)
{
    pstvnc_local_ui_t ui;

    pstvnc_local_ui_init(&ui);

    assert(
        !pstvnc_local_ui_close_osk(
            &ui));

    assert(
        ui.foreground ==
        PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP);

    assert(
        pstvnc_local_ui_generation(&ui) ==
        0u);

    assert(
        !pstvnc_local_ui_needs_present(
            &ui));
}

static void test_invalid_return_owner_fails_closed(void)
{
    pstvnc_local_ui_t ui;
    uint32_t generation;

    pstvnc_local_ui_init(&ui);

    assert(
        pstvnc_local_ui_open_osk(
            &ui));

    generation =
        pstvnc_local_ui_generation(&ui);

    /*
     * Simulate corrupted/speculative state. K4 must not silently create an
     * unsupported foreground transition.
     */
    ui.return_foreground =
        PSTVNC_LOCAL_UI_FOREGROUND_OSK;

    assert(
        !pstvnc_local_ui_close_osk(
            &ui));

    assert(
        ui.foreground ==
        PSTVNC_LOCAL_UI_FOREGROUND_OSK);

    assert(
        pstvnc_local_ui_generation(&ui) ==
        generation);
}

static void test_null_contract(void)
{
    pstvnc_local_ui_init(NULL);

    assert(
        !pstvnc_local_ui_open_osk(
            NULL));

    assert(
        !pstvnc_local_ui_close_osk(
            NULL));

    assert(
        !pstvnc_local_ui_input_is_quarantined(
            NULL));

    assert(
        !pstvnc_local_ui_complete_input_quarantine(
            NULL));

    assert(
        !pstvnc_local_ui_mark_local_change(
            NULL));

    assert(
        pstvnc_local_ui_generation(
            NULL) == 0u);

    assert(
        !pstvnc_local_ui_needs_present(
            NULL));

    assert(
        !pstvnc_local_ui_mark_presented(
            NULL,
            0u));
}

int main(void)
{
    test_initial_state_is_clean_desktop();
    test_open_osk_changes_foreground_and_quarantines();
    test_repeated_open_is_rejected_without_mutation();
    test_quarantine_requires_explicit_completion();
    test_local_change_advances_dirty_generation();
    test_stale_present_ack_cannot_clear_newer_change();
    test_close_returns_to_desktop_with_fresh_quarantine();
    test_close_outside_osk_is_rejected();
    test_invalid_return_owner_fails_closed();
    test_null_contract();

    puts("LOCAL_UI_TEST=PASS");
    return 0;
}

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "osk.h"

static void select_cell(
    pstvnc_osk_t *osk,
    pstvnc_osk_page_t page,
    unsigned int row,
    unsigned int col)
{
    osk->page = page;
    osk->row = row;
    osk->col = col;
}

static void test_historical_open_state(void)
{
    pstvnc_osk_t osk;

    memset(&osk, 0xa5, sizeof(osk));

    pstvnc_osk_reset_for_open(&osk);

    assert(osk.page == PSTVNC_OSK_PAGE_ABC);
    assert(osk.row == 1u);
    assert(osk.col == 0u);

    assert(!osk.shift);
    assert(!osk.ctrl);
    assert(!osk.alt);

    assert(PSTVNC_OSK_WIDTH == 600u);
    assert(PSTVNC_OSK_HEIGHT == 178u);
    assert(PSTVNC_OSK_ROWS == 5u);
}

static void test_exact_historical_row_lengths(void)
{
    static const unsigned int abc[5] = {
        12u, 13u, 11u, 11u, 11u
    };

    static const unsigned int func[5] = {
        12u, 6u, 3u, 3u, 11u
    };

    unsigned int row;

    for (row = 0; row < 5u; row++) {
        assert(
            pstvnc_osk_row_length(
                PSTVNC_OSK_PAGE_ABC,
                row) ==
            abc[row]);

        assert(
            pstvnc_osk_row_length(
                PSTVNC_OSK_PAGE_FUNC,
                row) ==
            func[row]);
    }

    assert(
        pstvnc_osk_row_length(
            PSTVNC_OSK_PAGE_COUNT,
            0u) == 0u);

    assert(
        pstvnc_osk_row_length(
            PSTVNC_OSK_PAGE_ABC,
            5u) == 0u);
}

static void test_exact_abc_rows_and_shift_layer(void)
{
    pstvnc_osk_t osk;

    pstvnc_osk_reset_for_open(&osk);

    select_cell(
        &osk,
        PSTVNC_OSK_PAGE_ABC,
        0u,
        0u);

    assert(
        pstvnc_osk_display_char(
            &osk, 0u, 0u) == '1');

    osk.shift = 1;

    assert(
        pstvnc_osk_display_char(
            &osk, 0u, 0u) == '!');

    osk.shift = 0;

    select_cell(
        &osk,
        PSTVNC_OSK_PAGE_ABC,
        1u,
        0u);

    assert(
        pstvnc_osk_display_char(
            &osk, 1u, 0u) == 'q');

    osk.shift = 1;

    assert(
        pstvnc_osk_display_char(
            &osk, 1u, 0u) == 'Q');

    osk.shift = 0;

    assert(
        pstvnc_osk_display_char(
            &osk, 1u, 10u) == '[');

    osk.shift = 1;

    assert(
        pstvnc_osk_display_char(
            &osk, 1u, 10u) == '{');

    select_cell(
        &osk,
        PSTVNC_OSK_PAGE_ABC,
        3u,
        10u);

    assert(
        pstvnc_osk_display_char(
            &osk, 3u, 10u) == '~');
}

static void test_historical_utility_labels(void)
{
    static const char *const labels[11] = {
        "ABC",
        "FUNC",
        "SHIFT",
        "CTRL",
        "ALT",
        "SPACE",
        "TAB",
        "BKSP",
        "DEL",
        "ENTER",
        "ESC"
    };

    pstvnc_osk_t osk;
    unsigned int col;

    pstvnc_osk_reset_for_open(&osk);

    for (col = 0; col < 11u; col++) {
        const char *label =
            pstvnc_osk_key_label(
                &osk,
                PSTVNC_OSK_UTILITY_ROW,
                col);

        assert(label != NULL);
        assert(strcmp(label, labels[col]) == 0);
    }
}

static void test_historical_func_labels(void)
{
    pstvnc_osk_t osk;

    pstvnc_osk_reset_for_open(&osk);
    osk.page = PSTVNC_OSK_PAGE_FUNC;

    assert(
        strcmp(
            pstvnc_osk_key_label(
                &osk, 0u, 0u),
            "F1") == 0);

    assert(
        strcmp(
            pstvnc_osk_key_label(
                &osk, 0u, 11u),
            "F12") == 0);

    assert(
        strcmp(
            pstvnc_osk_key_label(
                &osk, 1u, 0u),
            "HOME") == 0);

    assert(
        strcmp(
            pstvnc_osk_key_label(
                &osk, 1u, 5u),
            "CAPS") == 0);

    assert(
        strcmp(
            pstvnc_osk_key_label(
                &osk, 2u, 0u),
            "PRTSC") == 0);

    assert(
        strcmp(
            pstvnc_osk_key_label(
                &osk, 2u, 1u),
            "UP") == 0);

    assert(
        strcmp(
            pstvnc_osk_key_label(
                &osk, 2u, 2u),
            "PAUSE") == 0);

    assert(
        strcmp(
            pstvnc_osk_key_label(
                &osk, 3u, 0u),
            "LEFT") == 0);

    assert(
        strcmp(
            pstvnc_osk_key_label(
                &osk, 3u, 1u),
            "DOWN") == 0);

    assert(
        strcmp(
            pstvnc_osk_key_label(
                &osk, 3u, 2u),
            "RIGHT") == 0);
}

static void test_horizontal_wrap(void)
{
    pstvnc_osk_t osk;

    pstvnc_osk_reset_for_open(&osk);

    osk.row = 0u;
    osk.col = 0u;

    assert(
        pstvnc_osk_move_horizontal(
            &osk,
            -1));

    assert(osk.col == 11u);

    assert(
        pstvnc_osk_move_horizontal(
            &osk,
            1));

    assert(osk.col == 0u);
}

static void test_historical_vertical_mapping_is_preserved(void)
{
    pstvnc_osk_t osk;

    pstvnc_osk_reset_for_open(&osk);

    /*
     * Explicitly preserve the known behavior David identified for later
     * refinement rather than silently "fixing" it during reconstruction.
     *
     * ABC row0 has 12 keys and row1 has 13:
     *
     *   row0 col11
     *     down -> floor(11*13/12) = row1 col11
     *     up   -> floor(11*12/13) = row0 col10
     *
     * Therefore down/up does not return to the starting key.
     */
    osk.row = 0u;
    osk.col = 11u;

    assert(
        pstvnc_osk_move_vertical(
            &osk,
            1));

    assert(osk.row == 1u);
    assert(osk.col == 11u);

    assert(
        pstvnc_osk_move_vertical(
            &osk,
            -1));

    assert(osk.row == 0u);
    assert(osk.col == 10u);
}

static void test_vertical_row_wrap(void)
{
    pstvnc_osk_t osk;

    pstvnc_osk_reset_for_open(&osk);

    osk.row = 0u;
    osk.col = 0u;

    assert(
        pstvnc_osk_move_vertical(
            &osk,
            -1));

    assert(osk.row == 4u);
    assert(osk.col == 0u);

    assert(
        pstvnc_osk_move_vertical(
            &osk,
            1));

    assert(osk.row == 0u);
    assert(osk.col == 0u);
}

static void test_page_selectors_preserve_utility_selection(void)
{
    pstvnc_osk_t osk;
    pstvnc_osk_activation_t result;

    pstvnc_osk_reset_for_open(&osk);

    select_cell(
        &osk,
        PSTVNC_OSK_PAGE_ABC,
        PSTVNC_OSK_UTILITY_ROW,
        PSTVNC_OSK_UTILITY_FUNC);

    assert(
        pstvnc_osk_activate_selected(
            &osk,
            &result));

    assert(osk.page == PSTVNC_OSK_PAGE_FUNC);
    assert(osk.row == PSTVNC_OSK_UTILITY_ROW);
    assert(osk.col == PSTVNC_OSK_UTILITY_FUNC);
    assert(result.local_state_changed);
    assert(!result.produced_keyboard_tap);

    osk.col = PSTVNC_OSK_UTILITY_ABC;

    assert(
        pstvnc_osk_activate_selected(
            &osk,
            &result));

    assert(osk.page == PSTVNC_OSK_PAGE_ABC);
    assert(result.local_state_changed);
    assert(!result.produced_keyboard_tap);
}

static void test_modifier_toggles_are_local_state(void)
{
    pstvnc_osk_t osk;
    pstvnc_osk_activation_t result;

    pstvnc_osk_reset_for_open(&osk);
    osk.row = PSTVNC_OSK_UTILITY_ROW;

    osk.col = PSTVNC_OSK_UTILITY_SHIFT;

    assert(
        pstvnc_osk_activate_selected(
            &osk,
            &result));

    assert(osk.shift);
    assert(result.local_state_changed);
    assert(!result.produced_keyboard_tap);

    osk.col = PSTVNC_OSK_UTILITY_CTRL;

    assert(
        pstvnc_osk_activate_selected(
            &osk,
            &result));

    assert(osk.ctrl);

    osk.col = PSTVNC_OSK_UTILITY_ALT;

    assert(
        pstvnc_osk_activate_selected(
            &osk,
            &result));

    assert(osk.alt);

    assert(
        pstvnc_osk_clear_modifiers(
            &osk));

    assert(!osk.shift);
    assert(!osk.ctrl);
    assert(!osk.alt);

    assert(
        !pstvnc_osk_clear_modifiers(
            &osk));
}

static void test_printable_shift_is_target_not_real_shift(void)
{
    pstvnc_osk_t osk;
    pstvnc_osk_activation_t result;

    pstvnc_osk_reset_for_open(&osk);

    osk.shift = 1;
    osk.ctrl = 1;
    osk.alt = 1;

    select_cell(
        &osk,
        PSTVNC_OSK_PAGE_ABC,
        1u,
        0u);

    assert(
        pstvnc_osk_activate_selected(
            &osk,
            &result));

    assert(result.produced_keyboard_tap);
    assert(result.local_state_changed);

    /*
     * Shifted Q is the exact printable keysym 'Q'.
     * Real Ctrl/Alt remain around that target; real Shift does not.
     */
    assert(
        result.keyboard_tap.keysym ==
        (uint32_t)'Q');

    assert(
        result.keyboard_tap.modifiers ==
        (
            PSTVNC_KEYBOARD_MODIFIER_CTRL |
            PSTVNC_KEYBOARD_MODIFIER_ALT
        ));

    assert(!osk.shift);
    assert(!osk.ctrl);
    assert(!osk.alt);
}

static void test_plain_printable_is_lowercase(void)
{
    pstvnc_osk_t osk;
    pstvnc_osk_activation_t result;

    pstvnc_osk_reset_for_open(&osk);

    select_cell(
        &osk,
        PSTVNC_OSK_PAGE_ABC,
        1u,
        0u);

    assert(
        pstvnc_osk_activate_selected(
            &osk,
            &result));

    assert(result.produced_keyboard_tap);
    assert(result.keyboard_tap.keysym == (uint32_t)'q');
    assert(result.keyboard_tap.modifiers == 0u);
    assert(!result.local_state_changed);
}

static void test_func_page_uses_real_modifiers(void)
{
    pstvnc_osk_t osk;
    pstvnc_osk_activation_t result;

    pstvnc_osk_reset_for_open(&osk);

    osk.shift = 1;
    osk.ctrl = 1;
    osk.alt = 1;

    select_cell(
        &osk,
        PSTVNC_OSK_PAGE_FUNC,
        2u,
        1u);

    assert(
        pstvnc_osk_activate_selected(
            &osk,
            &result));

    assert(result.produced_keyboard_tap);

    /* Historical FUNC row2 col1 is the Up keysym. */
    assert(
        result.keyboard_tap.keysym ==
        0x0000ff52u);

    assert(
        result.keyboard_tap.modifiers ==
        (
            PSTVNC_KEYBOARD_MODIFIER_SHIFT |
            PSTVNC_KEYBOARD_MODIFIER_CTRL |
            PSTVNC_KEYBOARD_MODIFIER_ALT
        ));

    assert(!osk.shift);
    assert(!osk.ctrl);
    assert(!osk.alt);
}

static void test_shift_tab_is_true_modified_tab(void)
{
    pstvnc_osk_t osk;
    pstvnc_osk_activation_t result;

    pstvnc_osk_reset_for_open(&osk);

    osk.shift = 1;
    osk.ctrl = 1;

    select_cell(
        &osk,
        PSTVNC_OSK_PAGE_ABC,
        PSTVNC_OSK_UTILITY_ROW,
        PSTVNC_OSK_UTILITY_TAB);

    assert(
        pstvnc_osk_activate_selected(
            &osk,
            &result));

    assert(result.produced_keyboard_tap);

    assert(
        result.keyboard_tap.keysym ==
        0x0000ff09u);

    assert(
        result.keyboard_tap.modifiers ==
        (
            PSTVNC_KEYBOARD_MODIFIER_SHIFT |
            PSTVNC_KEYBOARD_MODIFIER_CTRL
        ));

    assert(!osk.shift);
    assert(!osk.ctrl);
    assert(!osk.alt);
}

static void test_utility_keysyms(void)
{
    static const struct {
        unsigned int col;
        uint32_t keysym;
    } cases[] = {
        { PSTVNC_OSK_UTILITY_SPACE,     0x00000020u },
        { PSTVNC_OSK_UTILITY_TAB,       0x0000ff09u },
        { PSTVNC_OSK_UTILITY_BACKSPACE, 0x0000ff08u },
        { PSTVNC_OSK_UTILITY_DELETE,    0x0000ffffu },
        { PSTVNC_OSK_UTILITY_ENTER,     0x0000ff0du },
        { PSTVNC_OSK_UTILITY_ESCAPE,    0x0000ff1bu }
    };

    pstvnc_osk_t osk;
    pstvnc_osk_activation_t result;
    unsigned int i;

    for (i = 0;
         i < sizeof(cases) / sizeof(cases[0]);
         i++) {

        pstvnc_osk_reset_for_open(&osk);

        select_cell(
            &osk,
            PSTVNC_OSK_PAGE_ABC,
            PSTVNC_OSK_UTILITY_ROW,
            cases[i].col);

        assert(
            pstvnc_osk_activate_selected(
                &osk,
                &result));

        assert(result.produced_keyboard_tap);

        assert(
            result.keyboard_tap.keysym ==
            cases[i].keysym);
    }
}

static void test_invalid_contract(void)
{
    pstvnc_osk_t osk;
    pstvnc_osk_activation_t result;

    pstvnc_osk_reset_for_open(NULL);

    assert(
        !pstvnc_osk_clear_modifiers(
            NULL));

    assert(
        !pstvnc_osk_move_horizontal(
            NULL,
            1));

    assert(
        !pstvnc_osk_move_vertical(
            NULL,
            1));

    assert(
        pstvnc_osk_display_char(
            NULL,
            0u,
            0u) == '\0');

    assert(
        pstvnc_osk_key_label(
            NULL,
            0u,
            0u) == NULL);

    pstvnc_osk_reset_for_open(&osk);

    osk.row = 99u;

    assert(
        !pstvnc_osk_activate_selected(
            &osk,
            &result));

    assert(
        !pstvnc_osk_activate_selected(
            NULL,
            &result));

    assert(
        !pstvnc_osk_activate_selected(
            &osk,
            NULL));
}

int main(void)
{
    test_historical_open_state();
    test_exact_historical_row_lengths();
    test_exact_abc_rows_and_shift_layer();
    test_historical_utility_labels();
    test_historical_func_labels();

    test_horizontal_wrap();
    test_historical_vertical_mapping_is_preserved();
    test_vertical_row_wrap();

    test_page_selectors_preserve_utility_selection();
    test_modifier_toggles_are_local_state();

    test_printable_shift_is_target_not_real_shift();
    test_plain_printable_is_lowercase();
    test_func_page_uses_real_modifiers();
    test_shift_tab_is_true_modified_tab();
    test_utility_keysyms();

    test_invalid_contract();

    puts("OSK_TEST=PASS");
    return 0;
}

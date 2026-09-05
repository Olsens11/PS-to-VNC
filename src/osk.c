/*
 * File synopsis:
 * Implements the clean historical B4A OSK behavior model without controller,
 * local-foreground, rendering, GS, application, or RFB ownership.
 *
 * The key inventory and navigation behavior are deliberately preserved for
 * Stage 2 restoration. Optional geometry-aware navigation refinement remains a
 * later usability change after historical behavior is hardware-qualified.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "osk.h"

/*
 * Historical ABC page.
 *
 * Letters are stored uppercase exactly as the adopted layout did. Display/key
 * interpretation converts them to lowercase while Shift is off.
 */
static const char *const osk_abc_rows[4] = {
    "1234567890-=",
    "QWERTYUIOP[]\\",
    "ASDFGHJKL;'",
    "ZXCVBNM,./`"
};

/*
 * Exact adopted row lengths.
 *
 * Row 4 is the common utility row on both pages.
 */
static const unsigned int
osk_page_row_lengths[PSTVNC_OSK_PAGE_COUNT][PSTVNC_OSK_ROWS] = {
    { 12u, 13u, 11u, 11u, 11u },
    { 12u,  6u,  3u,  3u, 11u }
};

/*
 * Historical FUNC page labels:
 *
 *   F1 ... F12
 *   HOME END PGUP PGDN INS CAPS
 *   PRTSC UP PAUSE
 *   LEFT DOWN RIGHT
 */
static const char *const osk_func_labels[4][12] = {
    {
        "F1", "F2", "F3", "F4",
        "F5", "F6", "F7", "F8",
        "F9", "F10", "F11", "F12"
    },
    {
        "HOME", "END", "PGUP", "PGDN",
        "INS", "CAPS",
        NULL, NULL, NULL, NULL, NULL, NULL
    },
    {
        "PRTSC", "UP", "PAUSE",
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    },
    {
        "LEFT", "DOWN", "RIGHT",
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    }
};

/*
 * Native X11 keysyms corresponding exactly to osk_func_labels.
 */
static const uint32_t osk_func_keysyms[4][12] = {
    {
        0x0000ffbeu, 0x0000ffbfu, 0x0000ffc0u, 0x0000ffc1u,
        0x0000ffc2u, 0x0000ffc3u, 0x0000ffc4u, 0x0000ffc5u,
        0x0000ffc6u, 0x0000ffc7u, 0x0000ffc8u, 0x0000ffc9u
    },
    {
        0x0000ff50u, /* Home */
        0x0000ff57u, /* End */
        0x0000ff55u, /* Page Up */
        0x0000ff56u, /* Page Down */
        0x0000ff63u, /* Insert */
        0x0000ffe5u, /* Caps Lock */
        0, 0, 0, 0, 0, 0
    },
    {
        0x0000ff61u, /* Print */
        0x0000ff52u, /* Up */
        0x0000ff13u, /* Pause */
        0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        0x0000ff51u, /* Left */
        0x0000ff54u, /* Down */
        0x0000ff53u, /* Right */
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

static const char *const
osk_special_labels[PSTVNC_OSK_UTILITY_KEY_COUNT] = {
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

static unsigned int osk_shifted_ascii(
    unsigned int c)
{
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 'A';

    switch (c) {
    case '1': return '!';
    case '2': return '@';
    case '3': return '#';
    case '4': return '$';
    case '5': return '%';
    case '6': return '^';
    case '7': return '&';
    case '8': return '*';
    case '9': return '(';
    case '0': return ')';
    case '-': return '_';
    case '=': return '+';
    case '[': return '{';
    case ']': return '}';
    case '\\': return '|';
    case ';': return ':';
    case '\'': return '"';
    case ',': return '<';
    case '.': return '>';
    case '/': return '?';
    case '`': return '~';
    default:
        return c;
    }
}

static uint32_t osk_func_keysym(
    unsigned int row,
    unsigned int col)
{
    if (row >= 4u ||
        col >=
            pstvnc_osk_row_length(
                PSTVNC_OSK_PAGE_FUNC,
                row))
        return 0;

    return osk_func_keysyms[row][col];
}

static unsigned int osk_real_modifier_mask(
    const pstvnc_osk_t *osk,
    int include_shift)
{
    unsigned int modifiers = 0;

    if (osk == NULL)
        return 0;

    if (include_shift && osk->shift)
        modifiers |=
            PSTVNC_KEYBOARD_MODIFIER_SHIFT;

    if (osk->ctrl)
        modifiers |=
            PSTVNC_KEYBOARD_MODIFIER_CTRL;

    if (osk->alt)
        modifiers |=
            PSTVNC_KEYBOARD_MODIFIER_ALT;

    return modifiers;
}

static void osk_activation_clear(
    pstvnc_osk_activation_t *result)
{
    if (result == NULL)
        return;

    memset(result, 0, sizeof(*result));
}

static void osk_consume_modifiers_after_key(
    pstvnc_osk_t *osk,
    pstvnc_osk_activation_t *result)
{
    int had_modifiers;

    had_modifiers =
        osk->shift ||
        osk->ctrl ||
        osk->alt;

    osk->shift = 0;
    osk->ctrl = 0;
    osk->alt = 0;

    if (had_modifiers)
        result->local_state_changed = 1;
}

void pstvnc_osk_reset_for_open(
    pstvnc_osk_t *osk)
{
    if (osk == NULL)
        return;

    osk->page = PSTVNC_OSK_PAGE_ABC;

    /*
     * Preserve the customized historical opening position: first key of the
     * QWERTY row rather than the number row.
     */
    osk->row = 1u;
    osk->col = 0u;

    osk->shift = 0;
    osk->ctrl = 0;
    osk->alt = 0;
}

int pstvnc_osk_clear_modifiers(
    pstvnc_osk_t *osk)
{
    int changed;

    if (osk == NULL)
        return 0;

    changed =
        osk->shift ||
        osk->ctrl ||
        osk->alt;

    osk->shift = 0;
    osk->ctrl = 0;
    osk->alt = 0;

    return changed ? 1 : 0;
}

int pstvnc_osk_toggle_shift_modifier(
    pstvnc_osk_t *osk)
{
    if (osk == NULL)
        return 0;

    osk->shift = !osk->shift;
    return 1;
}

int pstvnc_osk_activate_direct_key(
    pstvnc_osk_t *osk,
    uint32_t keysym,
    pstvnc_osk_activation_t *result)
{
    if (osk == NULL ||
        result == NULL)
        return 0;

    osk_activation_clear(result);

    if (keysym == 0)
        return 0;

    result->produced_keyboard_tap = 1;
    result->keyboard_tap.keysym = keysym;

    /*
     * Direct shortcuts correspond to FUNC/utility-style keys, so Shift is a
     * true remote modifier rather than an ABC printable-layer transform.
     */
    result->keyboard_tap.modifiers =
        osk_real_modifier_mask(
            osk,
            1);

    osk_consume_modifiers_after_key(
        osk,
        result);

    return 1;
}

unsigned int pstvnc_osk_row_length(
    pstvnc_osk_page_t page,
    unsigned int row)
{
    if ((unsigned int)page >=
            (unsigned int)PSTVNC_OSK_PAGE_COUNT ||
        row >= PSTVNC_OSK_ROWS)
        return 0;

    return osk_page_row_lengths[page][row];
}

int pstvnc_osk_move_horizontal(
    pstvnc_osk_t *osk,
    int direction)
{
    unsigned int count;
    unsigned int old_col;

    if (osk == NULL || direction == 0)
        return 0;

    count =
        pstvnc_osk_row_length(
            osk->page,
            osk->row);

    if (count == 0)
        return 0;

    if (osk->col >= count)
        return 0;

    old_col = osk->col;

    if (direction < 0) {
        if (osk->col == 0)
            osk->col = count - 1u;
        else
            osk->col--;
    } else {
        osk->col++;

        if (osk->col >= count)
            osk->col = 0;
    }

    return osk->col != old_col;
}

int pstvnc_osk_move_vertical(
    pstvnc_osk_t *osk,
    int direction)
{
    unsigned int old_count;
    unsigned int old_col;
    int new_row;
    unsigned int new_count;
    unsigned int new_col;

    if (osk == NULL || direction == 0)
        return 0;

    old_count =
        pstvnc_osk_row_length(
            osk->page,
            osk->row);

    if (old_count == 0 ||
        osk->col >= old_count)
        return 0;

    old_col = osk->col;

    new_row =
        (int)osk->row +
        (direction < 0 ? -1 : 1);

    if (new_row < 0)
        new_row =
            (int)PSTVNC_OSK_ROWS - 1;
    else if (new_row >=
             (int)PSTVNC_OSK_ROWS)
        new_row = 0;

    new_count =
        pstvnc_osk_row_length(
            osk->page,
            (unsigned int)new_row);

    if (new_count == 0)
        return 0;

    /*
     * Preserve the historical proportional-column rule exactly.
     *
     * This is intentionally NOT replaced with geometry-aware navigation in
     * the restoration stage. Example on ABC:
     *
     *   row0 col11 -> down -> row1 col11
     *              -> up   -> row0 col10
     *
     * That known non-reversibility is covered explicitly by host tests so a
     * later refinement will be a deliberate behavior change.
     */
    new_col =
        (old_col * new_count) /
        old_count;

    if (new_col >= new_count)
        new_col = new_count - 1u;

    osk->row = (unsigned int)new_row;
    osk->col = new_col;

    return 1;
}

char pstvnc_osk_display_char(
    const pstvnc_osk_t *osk,
    unsigned int row,
    unsigned int col)
{
    unsigned int c;

    if (osk == NULL ||
        osk->page != PSTVNC_OSK_PAGE_ABC ||
        row >= 4u ||
        col >=
            pstvnc_osk_row_length(
                PSTVNC_OSK_PAGE_ABC,
                row))
        return '\0';

    c =
        (unsigned int)
        (unsigned char)
        osk_abc_rows[row][col];

    /*
     * Alphabet rows are stored uppercase for convenient layout. Historical
     * behavior displays/sends lowercase with Shift off.
     */
    if (c >= 'A' && c <= 'Z')
        c = c - 'A' + 'a';

    if (osk->shift)
        c = osk_shifted_ascii(c);

    return (char)c;
}

const char *pstvnc_osk_key_label(
    const pstvnc_osk_t *osk,
    unsigned int row,
    unsigned int col)
{
    unsigned int count;

    if (osk == NULL ||
        row >= PSTVNC_OSK_ROWS)
        return NULL;

    count =
        pstvnc_osk_row_length(
            osk->page,
            row);

    if (col >= count)
        return NULL;

    if (row == PSTVNC_OSK_UTILITY_ROW)
        return osk_special_labels[col];

    if (osk->page == PSTVNC_OSK_PAGE_FUNC)
        return osk_func_labels[row][col];

    return NULL;
}

int pstvnc_osk_activate_selected(
    pstvnc_osk_t *osk,
    pstvnc_osk_activation_t *result)
{
    uint32_t keysym = 0;
    unsigned int modifiers = 0;
    unsigned int count;

    if (osk == NULL || result == NULL)
        return 0;

    osk_activation_clear(result);

    count =
        pstvnc_osk_row_length(
            osk->page,
            osk->row);

    if (count == 0 ||
        osk->col >= count)
        return 0;

    if (osk->row < PSTVNC_OSK_UTILITY_ROW) {
        if (osk->page == PSTVNC_OSK_PAGE_ABC) {
            char display_char =
                pstvnc_osk_display_char(
                    osk,
                    osk->row,
                    osk->col);

            if (display_char == '\0')
                return 0;

            keysym =
                (uint32_t)
                (unsigned char)
                display_char;

            /*
             * Printable Shift is already represented by the exact target
             * character. Only Ctrl/Alt remain real remote modifiers.
             */
            modifiers =
                osk_real_modifier_mask(
                    osk,
                    0);

        } else if (
            osk->page ==
            PSTVNC_OSK_PAGE_FUNC) {

            keysym =
                osk_func_keysym(
                    osk->row,
                    osk->col);

            if (keysym == 0)
                return 0;

            modifiers =
                osk_real_modifier_mask(
                    osk,
                    1);

        } else {
            return 0;
        }

        result->produced_keyboard_tap = 1;
        result->keyboard_tap.keysym = keysym;
        result->keyboard_tap.modifiers = modifiers;

        osk_consume_modifiers_after_key(
            osk,
            result);

        return 1;
    }

    /*
     * Permanent utility row:
     *
     *   ABC FUNC SHIFT CTRL ALT SPACE TAB BKSP DEL ENTER ESC
     */
    switch ((pstvnc_osk_utility_key_t)osk->col) {
    case PSTVNC_OSK_UTILITY_ABC:
        if (osk->page != PSTVNC_OSK_PAGE_ABC) {
            osk->page = PSTVNC_OSK_PAGE_ABC;
            result->local_state_changed = 1;
        }
        return 1;

    case PSTVNC_OSK_UTILITY_FUNC:
        if (osk->page != PSTVNC_OSK_PAGE_FUNC) {
            osk->page = PSTVNC_OSK_PAGE_FUNC;
            result->local_state_changed = 1;
        }
        return 1;

    case PSTVNC_OSK_UTILITY_SHIFT:
        osk->shift = !osk->shift;
        result->local_state_changed = 1;
        return 1;

    case PSTVNC_OSK_UTILITY_CTRL:
        osk->ctrl = !osk->ctrl;
        result->local_state_changed = 1;
        return 1;

    case PSTVNC_OSK_UTILITY_ALT:
        osk->alt = !osk->alt;
        result->local_state_changed = 1;
        return 1;

    case PSTVNC_OSK_UTILITY_SPACE:
        keysym = 0x00000020u;
        break;

    case PSTVNC_OSK_UTILITY_TAB:
        keysym = 0x0000ff09u;
        break;

    case PSTVNC_OSK_UTILITY_BACKSPACE:
        keysym = 0x0000ff08u;
        break;

    case PSTVNC_OSK_UTILITY_DELETE:
        keysym = 0x0000ffffu;
        break;

    case PSTVNC_OSK_UTILITY_ENTER:
        keysym = 0x0000ff0du;
        break;

    case PSTVNC_OSK_UTILITY_ESCAPE:
        keysym = 0x0000ff1bu;
        break;

    default:
        return 0;
    }

    /*
     * Historical FUNC/utility behavior treats one-shot Shift as a true remote
     * modifier here, preserving combinations such as Shift+Tab.
     */
    modifiers =
        osk_real_modifier_mask(
            osk,
            1);

    result->produced_keyboard_tap = 1;
    result->keyboard_tap.keysym = keysym;
    result->keyboard_tap.modifiers = modifiers;

    osk_consume_modifiers_after_key(
        osk,
        result);

    return 1;
}

/*
 * File synopsis:
 * Defines the clean Reconstruction Stage 2 on-screen-keyboard behavior owner.
 *
 * The adopted keyboard is intentionally reconstructed from the customized
 * historical B4A OSK rather than redesigned during restoration.
 *
 * This owner preserves:
 *
 *   - the 600x178, five-row OSK behavior/layout model;
 *   - ABC and FUNC pages;
 *   - the historical key inventory and row lengths;
 *   - the historical initial selection;
 *   - horizontal wrapping;
 *   - the historical proportional vertical-navigation rule;
 *   - one-shot Shift/Ctrl/Alt state;
 *   - printable Shift-layer versus real-modifier distinction.
 *
 * It does not:
 *
 *   - poll libpad;
 *   - own local foreground/modal policy;
 *   - draw pixels;
 *   - present GS frames;
 *   - serialize or write RFB;
 *   - interpret controller buttons.
 *
 * Context:
 *   docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md, B08/B09;
 *   working/b4a historical OSK authority;
 *   build/reconstruction/issue39/osk-historical-extraction.txt;
 *   GitHub Issue #39.
 *
 * Deferred refinement:
 * The historical proportional vertical mapping is preserved for initial Stage 2
 * restoration even though moving between unequal row lengths is not perfectly
 * reversible. Geometry-aware/canonical-grid navigation is deferred until after
 * the restored historical OSK is hardware-qualified.
 */

#ifndef PSTVNC_OSK_H
#define PSTVNC_OSK_H

#include <stdint.h>

#include "keyboard.h"

#define PSTVNC_OSK_WIDTH  600u
#define PSTVNC_OSK_HEIGHT 178u
#define PSTVNC_OSK_ROWS   5u

#define PSTVNC_OSK_UTILITY_ROW 4u
#define PSTVNC_OSK_UTILITY_KEY_COUNT 11u

typedef enum pstvnc_osk_page {
    PSTVNC_OSK_PAGE_ABC = 0,
    PSTVNC_OSK_PAGE_FUNC,
    PSTVNC_OSK_PAGE_COUNT
} pstvnc_osk_page_t;

/*
 * Stable indices of the historical permanent utility row:
 *
 *   ABC FUNC SHIFT CTRL ALT SPACE TAB BKSP DEL ENTER ESC
 */
typedef enum pstvnc_osk_utility_key {
    PSTVNC_OSK_UTILITY_ABC = 0,
    PSTVNC_OSK_UTILITY_FUNC,
    PSTVNC_OSK_UTILITY_SHIFT,
    PSTVNC_OSK_UTILITY_CTRL,
    PSTVNC_OSK_UTILITY_ALT,
    PSTVNC_OSK_UTILITY_SPACE,
    PSTVNC_OSK_UTILITY_TAB,
    PSTVNC_OSK_UTILITY_BACKSPACE,
    PSTVNC_OSK_UTILITY_DELETE,
    PSTVNC_OSK_UTILITY_ENTER,
    PSTVNC_OSK_UTILITY_ESCAPE
} pstvnc_osk_utility_key_t;

/*
 * Complete OSK-owned interaction state.
 *
 * Visibility/foreground ownership deliberately does not live here; K4 local_ui
 * owns whether OSK or desktop currently receives semantic interaction.
 */
typedef struct pstvnc_osk {
    pstvnc_osk_page_t page;

    unsigned int row;
    unsigned int col;

    int shift;
    int ctrl;
    int alt;
} pstvnc_osk_t;

/*
 * Result of activating the currently selected OSK key.
 *
 * local_state_changed tells application/UI composition whether the OSK's
 * visible state changed and therefore needs a local dirty generation update.
 *
 * produced_keyboard_tap identifies a complete semantic keyboard intent for the
 * existing K3 application/main publication path.
 */
typedef struct pstvnc_osk_activation {
    int local_state_changed;
    int produced_keyboard_tap;

    pstvnc_keyboard_tap_t keyboard_tap;
} pstvnc_osk_activation_t;

/*
 * Initialize/reset to the historical state used whenever the OSK opens:
 *
 *   ABC page
 *   row 1
 *   column 0
 *   Shift/Ctrl/Alt clear
 */
void pstvnc_osk_reset_for_open(
    pstvnc_osk_t *osk);

/*
 * Clear only one-shot modifier state.
 *
 * Closing the OSK uses this invariant; a subsequent open independently restores
 * page and selection through reset_for_open().
 *
 * Returns 1 if any modifier changed, otherwise 0.
 */
int pstvnc_osk_clear_modifiers(
    pstvnc_osk_t *osk);

/*
 * Toggle the OSK-owned one-shot Shift state.
 *
 * This supports the historical controller shortcut without allowing physical
 * controller vocabulary into the OSK behavior owner.
 */
int pstvnc_osk_toggle_shift_modifier(
    pstvnc_osk_t *osk);

/*
 * Produce one direct logical key using the current one-shot modifier state.
 *
 * This is the clean behavior seam for fixed OSK shortcuts such as Backspace,
 * Enter, and Tab. Shift/Ctrl/Alt are emitted as real modifiers and all one-shot
 * modifiers are consumed after the key action.
 */
int pstvnc_osk_activate_direct_key(
    pstvnc_osk_t *osk,
    uint32_t keysym,
    pstvnc_osk_activation_t *result);

/*
 * Return the adopted row length for page/row, or zero for invalid arguments.
 */
unsigned int pstvnc_osk_row_length(
    pstvnc_osk_page_t page,
    unsigned int row);

/*
 * Move within the current row with historical wrapping.
 *
 * direction < 0 moves left.
 * direction > 0 moves right.
 * direction == 0 performs no mutation.
 *
 * Returns 1 when selection changes.
 */
int pstvnc_osk_move_horizontal(
    pstvnc_osk_t *osk,
    int direction);

/*
 * Move vertically using the exact adopted historical rule:
 *
 *   new_col = old_col * new_row_length / old_row_length
 *
 * Rows wrap at top/bottom.
 *
 * This rule is intentionally preserved even though transitions between unequal
 * row lengths are not perfectly reversible.
 *
 * Returns 1 when selection changes.
 */
int pstvnc_osk_move_vertical(
    pstvnc_osk_t *osk,
    int direction);

/*
 * Return the exact printable character displayed by one ABC-page character
 * cell after current Shift-layer interpretation.
 *
 * Returns '\0' for non-printable/invalid cells.
 */
char pstvnc_osk_display_char(
    const pstvnc_osk_t *osk,
    unsigned int row,
    unsigned int col);

/*
 * Return the historical textual label for a FUNC-page or permanent utility
 * key. ABC printable character cells return NULL; rendering should use
 * pstvnc_osk_display_char() there.
 */
const char *pstvnc_osk_key_label(
    const pstvnc_osk_t *osk,
    unsigned int row,
    unsigned int col);

/*
 * Activate the current selection.
 *
 * Page selectors and modifier keys mutate only OSK-local state.
 *
 * Actual keys produce one compact pstvnc_keyboard_tap_t. Shift on ABC
 * printable keys selects the exact shifted printable keysym and is NOT emitted
 * as a real remote Shift modifier. Ctrl/Alt remain real modifiers.
 *
 * FUNC and utility keys treat Shift/Ctrl/Alt as real modifiers.
 *
 * Every actual key action consumes all one-shot modifiers.
 *
 * Returns 1 for a valid activation, otherwise 0 with result cleared.
 */
int pstvnc_osk_activate_selected(
    pstvnc_osk_t *osk,
    pstvnc_osk_activation_t *result);

#endif /* PSTVNC_OSK_H */

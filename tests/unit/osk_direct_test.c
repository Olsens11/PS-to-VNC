#include <stdio.h>
#include <string.h>

#include "osk.h"

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, \
                "osk_direct_test:%d: CHECK failed: %s\n", \
                __LINE__, #condition); \
            return 1; \
        } \
    } while (0)

int main(void)
{
    pstvnc_osk_t osk;
    pstvnc_osk_activation_t result;

    pstvnc_osk_reset_for_open(&osk);

    CHECK(
        pstvnc_osk_toggle_shift_modifier(
            &osk));

    CHECK(osk.shift == 1);

    memset(&result, 0, sizeof(result));

    /*
     * Shift+Tab must use a real Shift modifier rather than printable-layer
     * transformation.
     */
    CHECK(
        pstvnc_osk_activate_direct_key(
            &osk,
            0x0000ff09u,
            &result));

    CHECK(result.produced_keyboard_tap == 1);
    CHECK(result.keyboard_tap.keysym == 0x0000ff09u);

    CHECK(
        result.keyboard_tap.modifiers ==
        PSTVNC_KEYBOARD_MODIFIER_SHIFT);

    CHECK(result.local_state_changed == 1);
    CHECK(osk.shift == 0);
    CHECK(osk.ctrl == 0);
    CHECK(osk.alt == 0);

    memset(&result, 0xff, sizeof(result));

    CHECK(
        !pstvnc_osk_activate_direct_key(
            &osk,
            0,
            &result));

    CHECK(result.produced_keyboard_tap == 0);
    CHECK(result.local_state_changed == 0);

    memset(&result, 0, sizeof(result));

    CHECK(
        pstvnc_osk_activate_direct_key(
            &osk,
            0x0000ff08u,
            &result));

    CHECK(result.produced_keyboard_tap == 1);
    CHECK(result.keyboard_tap.keysym == 0x0000ff08u);
    CHECK(result.keyboard_tap.modifiers == 0);
    CHECK(result.local_state_changed == 0);

    puts("OSK_DIRECT_TEST=PASS");
    return 0;
}

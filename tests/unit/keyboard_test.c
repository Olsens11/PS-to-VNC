#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "keyboard.h"

static int failures = 0;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

static void check_event(
    const pstvnc_keyboard_sequence_t *sequence,
    unsigned int index,
    int expected_down,
    uint32_t expected_keysym)
{
    CHECK(sequence != NULL);

    if (sequence == NULL)
        return;

    CHECK(index < sequence->event_count);

    if (index >= sequence->event_count)
        return;

    CHECK(
        sequence->events[index].down ==
        expected_down);

    CHECK(
        sequence->events[index].keysym ==
        expected_keysym);
}

static void test_plain_tap(void)
{
    pstvnc_keyboard_sequence_t sequence;

    memset(&sequence, 0xa5, sizeof(sequence));

    CHECK(
        pstvnc_keyboard_build_tap_sequence(
            0x00000061u,
            0,
            &sequence));

    CHECK(sequence.event_count == 2);

    check_event(
        &sequence,
        0,
        1,
        0x00000061u);

    check_event(
        &sequence,
        1,
        0,
        0x00000061u);
}

static void test_single_real_modifier(void)
{
    pstvnc_keyboard_sequence_t sequence;

    memset(&sequence, 0, sizeof(sequence));

    CHECK(
        pstvnc_keyboard_build_tap_sequence(
            0x0000ff09u,
            PSTVNC_KEYBOARD_MODIFIER_SHIFT,
            &sequence));

    CHECK(sequence.event_count == 4);

    check_event(
        &sequence,
        0,
        1,
        PSTVNC_KEYBOARD_KEYSYM_SHIFT_L);

    check_event(
        &sequence,
        1,
        1,
        0x0000ff09u);

    check_event(
        &sequence,
        2,
        0,
        0x0000ff09u);

    check_event(
        &sequence,
        3,
        0,
        PSTVNC_KEYBOARD_KEYSYM_SHIFT_L);
}

static void test_all_modifiers_are_nested_and_balanced(void)
{
    pstvnc_keyboard_sequence_t sequence;
    unsigned int modifiers =
        PSTVNC_KEYBOARD_MODIFIER_SHIFT |
        PSTVNC_KEYBOARD_MODIFIER_CTRL |
        PSTVNC_KEYBOARD_MODIFIER_ALT;

    memset(&sequence, 0, sizeof(sequence));

    CHECK(
        pstvnc_keyboard_build_tap_sequence(
            0x0000ff0du,
            modifiers,
            &sequence));

    CHECK(
        PSTVNC_KEYBOARD_SEQUENCE_MAX_EVENTS ==
        8u);

    CHECK(sequence.event_count == 8);

    check_event(
        &sequence,
        0,
        1,
        PSTVNC_KEYBOARD_KEYSYM_SHIFT_L);

    check_event(
        &sequence,
        1,
        1,
        PSTVNC_KEYBOARD_KEYSYM_CONTROL_L);

    check_event(
        &sequence,
        2,
        1,
        PSTVNC_KEYBOARD_KEYSYM_ALT_L);

    check_event(
        &sequence,
        3,
        1,
        0x0000ff0du);

    check_event(
        &sequence,
        4,
        0,
        0x0000ff0du);

    check_event(
        &sequence,
        5,
        0,
        PSTVNC_KEYBOARD_KEYSYM_ALT_L);

    check_event(
        &sequence,
        6,
        0,
        PSTVNC_KEYBOARD_KEYSYM_CONTROL_L);

    check_event(
        &sequence,
        7,
        0,
        PSTVNC_KEYBOARD_KEYSYM_SHIFT_L);
}

static void test_full_32bit_target_keysym_is_preserved(void)
{
    pstvnc_keyboard_sequence_t sequence;

    memset(&sequence, 0, sizeof(sequence));

    CHECK(
        pstvnc_keyboard_build_tap_sequence(
            0x010000e9u,
            PSTVNC_KEYBOARD_MODIFIER_CTRL,
            &sequence));

    CHECK(sequence.event_count == 4);

    check_event(
        &sequence,
        1,
        1,
        0x010000e9u);

    check_event(
        &sequence,
        2,
        0,
        0x010000e9u);
}

static void test_modifier_constants_are_native_x11_values(void)
{
    CHECK(
        PSTVNC_KEYBOARD_KEYSYM_SHIFT_L ==
        0x0000ffe1u);

    CHECK(
        PSTVNC_KEYBOARD_KEYSYM_CONTROL_L ==
        0x0000ffe3u);

    CHECK(
        PSTVNC_KEYBOARD_KEYSYM_ALT_L ==
        0x0000ffe9u);
}

static void test_invalid_modifier_mask_publishes_nothing(void)
{
    pstvnc_keyboard_sequence_t sequence;

    memset(&sequence, 0xa5, sizeof(sequence));

    CHECK(
        !pstvnc_keyboard_build_tap_sequence(
            0x00000061u,
            PSTVNC_KEYBOARD_MODIFIER_MASK | 0x80u,
            &sequence));

    CHECK(sequence.event_count == 0);

    CHECK(
        !pstvnc_keyboard_build_tap_sequence(
            0x00000061u,
            0,
            NULL));
}

int main(void)
{
    test_plain_tap();
    test_single_real_modifier();
    test_all_modifiers_are_nested_and_balanced();
    test_full_32bit_target_keysym_is_preserved();
    test_modifier_constants_are_native_x11_values();
    test_invalid_modifier_mask_publishes_nothing();

    if (failures != 0) {
        fprintf(
            stderr,
            "keyboard_test: %d failure(s)\n",
            failures);

        return 1;
    }

    puts("keyboard_test: PASS");
    return 0;
}

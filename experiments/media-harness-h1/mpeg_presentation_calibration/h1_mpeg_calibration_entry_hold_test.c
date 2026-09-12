/*
 * File synopsis:
 * Strict host contract for the experiment-local held START+SELECT entry timer.
 */
#include "h1_mpeg_calibration_entry_hold.h"

#include <assert.h>
#include <stdio.h>

static void test_hold_promotes_at_exact_deadline(void)
{
    pstvnc_h1_mpeg_calibration_entry_hold_t hold;
    int consume = 0;
    int activate = 0;

    pstvnc_h1_mpeg_calibration_entry_hold_init(&hold);

    assert(pstvnc_h1_mpeg_calibration_entry_hold_observe(
        &hold, UINT64_C(1000000), 1, 1, &consume));
    assert(consume);
    assert(hold.armed);

    assert(pstvnc_h1_mpeg_calibration_entry_hold_poll(
        &hold, UINT64_C(1749999), 1, &activate));
    assert(!activate);
    assert(hold.armed);

    assert(pstvnc_h1_mpeg_calibration_entry_hold_poll(
        &hold, UINT64_C(1750000), 1, &activate));
    assert(activate);
    assert(!hold.armed);

    activate = 1;
    assert(pstvnc_h1_mpeg_calibration_entry_hold_poll(
        &hold, UINT64_C(2000000), 1, &activate));
    assert(!activate);
}

static void test_transitions_do_not_reset_held_deadline(void)
{
    pstvnc_h1_mpeg_calibration_entry_hold_t hold;
    int consume = 0;
    int activate = 0;

    pstvnc_h1_mpeg_calibration_entry_hold_init(&hold);

    assert(pstvnc_h1_mpeg_calibration_entry_hold_observe(
        &hold, UINT64_C(500), 1, 1, &consume));
    assert(consume);

    consume = 0;
    assert(pstvnc_h1_mpeg_calibration_entry_hold_observe(
        &hold, UINT64_C(500000), 1, 1, &consume));
    assert(consume);
    assert(hold.started_us == UINT64_C(500));

    assert(pstvnc_h1_mpeg_calibration_entry_hold_poll(
        &hold,
        UINT64_C(500) + PSTVNC_H1_MPEG_CALIBRATION_ENTRY_HOLD_US,
        1,
        &activate));
    assert(activate);
}

static void test_release_before_deadline_cancels_and_is_consumed(void)
{
    pstvnc_h1_mpeg_calibration_entry_hold_t hold;
    int consume = 0;
    int activate = 0;

    pstvnc_h1_mpeg_calibration_entry_hold_init(&hold);

    assert(pstvnc_h1_mpeg_calibration_entry_hold_observe(
        &hold, UINT64_C(100), 1, 1, &consume));
    assert(consume);

    consume = 0;
    assert(pstvnc_h1_mpeg_calibration_entry_hold_observe(
        &hold, UINT64_C(200), 1, 0, &consume));
    assert(consume);
    assert(!hold.armed);

    assert(pstvnc_h1_mpeg_calibration_entry_hold_poll(
        &hold, UINT64_C(900000), 1, &activate));
    assert(!activate);
}

static void test_unavailable_foreground_does_not_steal_osk_input(void)
{
    pstvnc_h1_mpeg_calibration_entry_hold_t hold;
    int consume = 1;
    int activate = 1;

    pstvnc_h1_mpeg_calibration_entry_hold_init(&hold);

    assert(pstvnc_h1_mpeg_calibration_entry_hold_observe(
        &hold, UINT64_C(100), 0, 1, &consume));
    assert(!consume);
    assert(!hold.armed);

    assert(pstvnc_h1_mpeg_calibration_entry_hold_poll(
        &hold, UINT64_C(900000), 0, &activate));
    assert(!activate);
}

static void test_backwards_time_rearms_instead_of_promoting(void)
{
    pstvnc_h1_mpeg_calibration_entry_hold_t hold;
    int consume = 0;
    int activate = 0;

    pstvnc_h1_mpeg_calibration_entry_hold_init(&hold);

    assert(pstvnc_h1_mpeg_calibration_entry_hold_observe(
        &hold, UINT64_C(1000), 1, 1, &consume));
    assert(consume);

    assert(pstvnc_h1_mpeg_calibration_entry_hold_poll(
        &hold, UINT64_C(900), 1, &activate));
    assert(!activate);
    assert(hold.armed);
    assert(hold.started_us == UINT64_C(900));
}

int main(void)
{
    test_hold_promotes_at_exact_deadline();
    test_transitions_do_not_reset_held_deadline();
    test_release_before_deadline_cancels_and_is_consumed();
    test_unavailable_foreground_does_not_steal_osk_input();
    test_backwards_time_rearms_instead_of_promoting();

    puts("MPEG_CALIBRATION_ENTRY_HOLD_HOST_TEST=PASS");
    return 0;
}

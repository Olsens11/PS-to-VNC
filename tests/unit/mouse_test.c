/*
 * File synopsis:
 * Host-tests the pure Stage #38 mouse: cursor motion, L/R clicks, D-pad
 * precision/acceleration, L3-style wheel-click toggle behavior, wheel motion,
 * clamping, fractional state, and repeat semantics without libpad or RFB.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "mouse.h"

static pstvnc_mouse_input_t neutral_input(void)
{
    pstvnc_mouse_input_t input;

    memset(&input, 0, sizeof(input));

    input.dpad_mode =
        PSTVNC_MOUSE_DPAD_POINTER;

    input.stick_available = 1;
    input.stick_x = 128;
    input.stick_y = 128;

    return input;
}

static pstvnc_mouse_update_t sample(
    pstvnc_mouse_t *mouse,
    const pstvnc_mouse_input_t *input)
{
    pstvnc_mouse_update_t update;

    assert(
        pstvnc_mouse_update(
            mouse,
            input,
            &update));

    return update;
}

static void test_init_and_validation(void)
{
    pstvnc_mouse_t mouse;

    assert(pstvnc_mouse_init(&mouse, 640, 480));

    assert(mouse.cursor_x == 320);
    assert(mouse.cursor_y == 240);
    assert(mouse.click_buttons == 0);
    assert(!mouse.wheel_mode_enabled);

    assert(!pstvnc_mouse_init(NULL, 640, 480));
    assert(!pstvnc_mouse_init(&mouse, 0, 480));
    assert(!pstvnc_mouse_init(&mouse, 640, 0));
}

static void test_dpad_precision_and_direction_change(void)
{
    pstvnc_mouse_t mouse;
    pstvnc_mouse_input_t input = neutral_input();
    pstvnc_mouse_update_t update;
    unsigned int i;

    assert(pstvnc_mouse_init(&mouse, 640, 480));

    input.dpad_directions =
        PSTVNC_MOUSE_DIRECTION_RIGHT;

    update = sample(&mouse, &input);

    assert(update.pointer_changed);
    assert(update.cursor_x == 322);
    assert(update.cursor_y == 240);

    for (i = 0; i < 17; i++) {
        update = sample(&mouse, &input);
        assert(update.cursor_x == 322);
    }

    update = sample(&mouse, &input);
    assert(update.cursor_x == 324);

    for (i = 0; i < 3; i++) {
        update = sample(&mouse, &input);
        assert(update.cursor_x == 324);
    }

    update = sample(&mouse, &input);
    assert(update.cursor_x == 326);

    input.dpad_directions =
        PSTVNC_MOUSE_DIRECTION_LEFT;

    update = sample(&mouse, &input);
    assert(update.cursor_x == 324);

    input.dpad_directions = 0;
    (void)sample(&mouse, &input);

    input.dpad_directions =
        PSTVNC_MOUSE_DIRECTION_UP |
        PSTVNC_MOUSE_DIRECTION_RIGHT;

    update = sample(&mouse, &input);

    assert(update.cursor_x == 326);
    assert(update.cursor_y == 238);
}

static void test_dpad_acceleration_and_wheel_boundary(void)
{
    pstvnc_mouse_t mouse;
    pstvnc_mouse_input_t input = neutral_input();
    pstvnc_mouse_update_t update;
    unsigned int i;
    unsigned int cursor_before_wheel;

    assert(pstvnc_mouse_init(&mouse, 2000, 480));

    input.dpad_directions =
        PSTVNC_MOUSE_DIRECTION_RIGHT;

    update = sample(&mouse, &input);
    assert(update.cursor_x == 1002);

    for (i = 0; i < 100; i++)
        update = sample(&mouse, &input);

    assert(update.cursor_x > 1030);

    cursor_before_wheel = update.cursor_x;

    input.dpad_mode =
        PSTVNC_MOUSE_DPAD_WHEEL;

    update = sample(&mouse, &input);

    assert(update.cursor_x == cursor_before_wheel);
    assert(
        update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_RIGHT);

    /*
     * Returning the still-held D-pad to cursor ownership is a fresh
     * two-pixel gesture rather than continuation of old acceleration.
     */
    input.dpad_mode =
        PSTVNC_MOUSE_DPAD_POINTER;

    update = sample(&mouse, &input);

    assert(
        update.cursor_x ==
        cursor_before_wheel + 2);
}

static void test_analog_deadzone_fraction_and_full_speed(void)
{
    pstvnc_mouse_t mouse;
    pstvnc_mouse_input_t input = neutral_input();
    pstvnc_mouse_update_t update;
    unsigned int i;

    assert(pstvnc_mouse_init(&mouse, 2000, 480));

    input.stick_x = 138;

    for (i = 0; i < 200; i++)
        update = sample(&mouse, &input);

    assert(update.cursor_x == 1000);

    input.stick_x = 139;

    for (i = 0; i < 127; i++) {
        update = sample(&mouse, &input);
        assert(update.cursor_x == 1000);
    }

    update = sample(&mouse, &input);
    assert(update.cursor_x == 1001);

    input.stick_x = 128;
    (void)sample(&mouse, &input);

    input.stick_x = 139;

    for (i = 0; i < 127; i++) {
        update = sample(&mouse, &input);
        assert(update.cursor_x == 1001);
    }

    input.stick_x = 255;

    update = sample(&mouse, &input);
    assert(update.cursor_x == 1007);
}

static void test_clamping_and_clicks(void)
{
    pstvnc_mouse_t mouse;
    pstvnc_mouse_input_t input = neutral_input();
    pstvnc_mouse_update_t update;
    unsigned int i;

    assert(pstvnc_mouse_init(&mouse, 5, 3));

    input.stick_x = 0;
    input.stick_y = 0;

    for (i = 0; i < 10; i++)
        update = sample(&mouse, &input);

    assert(update.cursor_x == 0);
    assert(update.cursor_y == 0);

    input.stick_x = 255;
    input.stick_y = 255;

    for (i = 0; i < 10; i++)
        update = sample(&mouse, &input);

    assert(update.cursor_x == 4);
    assert(update.cursor_y == 2);

    input.stick_x = 128;
    input.stick_y = 128;

    input.click_buttons =
        PSTVNC_MOUSE_BUTTON_LCLICK;

    update = sample(&mouse, &input);

    assert(update.pointer_changed);
    assert(
        update.click_buttons ==
        PSTVNC_MOUSE_BUTTON_LCLICK);

    input.click_buttons =
        PSTVNC_MOUSE_BUTTON_LCLICK |
        PSTVNC_MOUSE_BUTTON_RCLICK;

    update = sample(&mouse, &input);

    assert(update.pointer_changed);

    assert(
        update.click_buttons ==
        (PSTVNC_MOUSE_BUTTON_LCLICK |
         PSTVNC_MOUSE_BUTTON_RCLICK));

    update = sample(&mouse, &input);
    assert(!update.pointer_changed);
}

static void test_dpad_wheel_priority_and_repeat(void)
{
    pstvnc_mouse_t mouse;
    pstvnc_mouse_input_t input = neutral_input();
    pstvnc_mouse_update_t update;
    unsigned int i;

    assert(pstvnc_mouse_init(&mouse, 640, 480));

    input.dpad_mode =
        PSTVNC_MOUSE_DPAD_WHEEL;

    input.dpad_directions =
        PSTVNC_MOUSE_DIRECTION_UP |
        PSTVNC_MOUSE_DIRECTION_RIGHT;

    update = sample(&mouse, &input);

    assert(
        update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_UP);

    /*
     * Fixed D-pad wheel repeat is five intervening polls.
     */
    for (i = 0; i < 5; i++) {
        update = sample(&mouse, &input);

        assert(
            update.wheel_direction ==
            PSTVNC_MOUSE_WHEEL_NONE);
    }

    update = sample(&mouse, &input);

    assert(
        update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_UP);

    /*
     * Wheel direction changes fire immediately.
     */
    input.dpad_directions =
        PSTVNC_MOUSE_DIRECTION_LEFT;

    update = sample(&mouse, &input);

    assert(
        update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_LEFT);
}

static void test_wheel_click_toggles_analog_wheel(void)
{
    pstvnc_mouse_t mouse;
    pstvnc_mouse_input_t input = neutral_input();
    pstvnc_mouse_update_t update;
    unsigned int i;
    unsigned int cursor_before_wheel;

    assert(pstvnc_mouse_init(&mouse, 2000, 480));

    /*
     * Build almost one pixel of fractional cursor movement.
     */
    input.stick_x = 139;

    for (i = 0; i < 127; i++)
        update = sample(&mouse, &input);

    assert(update.cursor_x == 1000);

    cursor_before_wheel = update.cursor_x;

    /*
     * Clicking the wheel toggles wheel mode and is itself a clean boundary.
     * Even with the stick fully deflected, the click poll produces neither
     * cursor movement nor a wheel notch.
     */
    input.wheel_click_pressed = 1;
    input.stick_x = 255;

    update = sample(&mouse, &input);

    assert(mouse.wheel_mode_enabled);
    assert(update.cursor_x == cursor_before_wheel);

    assert(
        update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_NONE);

    input.wheel_click_pressed = 0;

    /*
     * The following poll uses the same stick as a wheel.
     */
    update = sample(&mouse, &input);

    assert(update.cursor_x == cursor_before_wheel);

    assert(
        update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_RIGHT);

    /*
     * Full deflection yields two quiet polls before the next notch.
     */
    update = sample(&mouse, &input);

    assert(
        update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_NONE);

    update = sample(&mouse, &input);

    assert(
        update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_NONE);

    update = sample(&mouse, &input);

    assert(
        update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_RIGHT);

    /*
     * Click again to return to cursor mode. That click poll is also quiet.
     */
    input.wheel_click_pressed = 1;
    input.stick_x = 139;

    update = sample(&mouse, &input);

    assert(!mouse.wheel_mode_enabled);
    assert(update.cursor_x == cursor_before_wheel);

    assert(
        update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_NONE);

    input.wheel_click_pressed = 0;

    /*
     * Fractional pointer motion starts cleanly from zero after leaving wheel
     * mode; no pre-wheel remainder survives.
     */
    for (i = 0; i < 127; i++) {
        update = sample(&mouse, &input);
        assert(update.cursor_x == cursor_before_wheel);
    }

    update = sample(&mouse, &input);

    assert(
        update.cursor_x ==
        cursor_before_wheel + 1);
}

static void test_dpad_wheel_beats_analog_wheel(void)
{
    pstvnc_mouse_t mouse;
    pstvnc_mouse_input_t input = neutral_input();
    pstvnc_mouse_update_t update;

    assert(pstvnc_mouse_init(&mouse, 640, 480));

    /*
     * Enter analog wheel mode first.
     */
    input.wheel_click_pressed = 1;
    (void)sample(&mouse, &input);

    assert(mouse.wheel_mode_enabled);

    input.wheel_click_pressed = 0;

    /*
     * Triangle+D-pad wheel input remains independently usable and wins when
     * both wheel sources are active in the same sample.
     */
    input.dpad_mode =
        PSTVNC_MOUSE_DPAD_WHEEL;

    input.dpad_directions =
        PSTVNC_MOUSE_DIRECTION_DOWN;

    input.stick_x = 0;
    input.stick_y = 128;

    update = sample(&mouse, &input);

    assert(
        update.wheel_direction ==
        PSTVNC_MOUSE_WHEEL_DOWN);
}

static void test_derived_reset_preserves_remote_pointer_state(void)
{
    pstvnc_mouse_t mouse;
    pstvnc_mouse_input_t input = neutral_input();
    pstvnc_mouse_update_t update;
    unsigned int cursor_x;
    unsigned int cursor_y;

    assert(pstvnc_mouse_init(&mouse, 640, 480));

    /*
     * Establish an ordinary remote pointer-button state.
     */
    input.click_buttons =
        PSTVNC_MOUSE_BUTTON_LCLICK;

    update = sample(&mouse, &input);

    assert(update.pointer_changed);

    assert(
        update.click_buttons ==
        PSTVNC_MOUSE_BUTTON_LCLICK);

    cursor_x = update.cursor_x;
    cursor_y = update.cursor_y;

    /*
     * A derived-state reset is not itself a synthetic remote button release.
     * Physical/input ownership and already-published remote pointer state are
     * separate concerns.
     */
    pstvnc_mouse_reset_derived(&mouse);

    assert(mouse.cursor_x == (int)cursor_x);
    assert(mouse.cursor_y == (int)cursor_y);

    assert(
        mouse.click_buttons ==
        PSTVNC_MOUSE_BUTTON_LCLICK);

    /*
     * Re-observing the same held click therefore publishes nothing new.
     */
    update = sample(&mouse, &input);

    assert(!update.pointer_changed);

    assert(
        update.click_buttons ==
        PSTVNC_MOUSE_BUTTON_LCLICK);

    /*
     * An actual routed release remains observable and produces the required
     * remote pointer-state change.
     */
    input.click_buttons = 0;

    update = sample(&mouse, &input);

    assert(update.pointer_changed);
    assert(update.click_buttons == 0);
    assert(update.cursor_x == cursor_x);
    assert(update.cursor_y == cursor_y);
}

static void test_transient_history_reset_preserves_wheel_mode(void)
{
    pstvnc_mouse_t mouse;
    pstvnc_mouse_input_t input = neutral_input();

    assert(pstvnc_mouse_init(&mouse, 640u, 480u));

    /*
     * Enter the user-selected persistent analog-wheel mode normally.
     */
    input.wheel_click_pressed = 1;
    (void)sample(&mouse, &input);

    assert(mouse.wheel_mode_enabled);

    /*
     * Seed every transient response-history family so the test proves that
     * preserving the mode does not accidentally preserve stale motion/repeat
     * state.
     */
    mouse.dpad_hold_direction =
        PSTVNC_MOUSE_DIRECTION_RIGHT;
    mouse.dpad_hold_ticks = 7u;
    mouse.dpad_x_q8 = 11;
    mouse.dpad_y_q8 = -13;

    mouse.analog_x_q8 = 17;
    mouse.analog_y_q8 = -19;

    mouse.last_wheel_direction =
        PSTVNC_MOUSE_WHEEL_DOWN;
    mouse.wheel_repeat_countdown = 3u;

    mouse.cursor_x = 123;
    mouse.cursor_y = 234;
    mouse.click_buttons =
        PSTVNC_MOUSE_BUTTON_LCLICK;

    pstvnc_mouse_reset_transient_history(&mouse);

    /*
     * Foreground-style suspension preserves durable interaction state.
     */
    assert(mouse.wheel_mode_enabled);
    assert(mouse.cursor_x == 123);
    assert(mouse.cursor_y == 234);
    assert(
        mouse.click_buttons ==
        PSTVNC_MOUSE_BUTTON_LCLICK);

    /*
     * But no pre-boundary movement or repeat history may resume.
     */
    assert(mouse.dpad_hold_direction == 0);
    assert(mouse.dpad_hold_ticks == 0u);
    assert(mouse.dpad_x_q8 == 0);
    assert(mouse.dpad_y_q8 == 0);
    assert(mouse.analog_x_q8 == 0);
    assert(mouse.analog_y_q8 == 0);

    assert(
        mouse.last_wheel_direction ==
        PSTVNC_MOUSE_WHEEL_NONE);

    assert(mouse.wheel_repeat_countdown == 0u);
}


static void test_explicit_derived_reset(void)
{
    pstvnc_mouse_t mouse;
    pstvnc_mouse_input_t input = neutral_input();
    pstvnc_mouse_update_t update;
    unsigned int i;

    assert(pstvnc_mouse_init(&mouse, 2000, 480));

    /*
     * Establish wheel mode, then prove a hard derived-state reset leaves it.
     */
    input.wheel_click_pressed = 1;
    (void)sample(&mouse, &input);

    assert(mouse.wheel_mode_enabled);

    pstvnc_mouse_reset_derived(&mouse);

    assert(!mouse.wheel_mode_enabled);

    input.wheel_click_pressed = 0;
    input.stick_x = 139;

    /*
     * Fractional pointer state also restarts from zero.
     */
    for (i = 0; i < 127; i++) {
        update = sample(&mouse, &input);
        assert(update.cursor_x == 1000);
    }

    update = sample(&mouse, &input);
    assert(update.cursor_x == 1001);
}


static void test_published_state_rebase(void)
{
    pstvnc_mouse_t mouse;
    pstvnc_mouse_input_t input;
    pstvnc_mouse_update_t update;

    assert(pstvnc_mouse_init(&mouse, 640u, 480u));

    memset(&input, 0, sizeof(input));

    /*
     * Deliberately advance local interpreted state away from the state the
     * application will later declare authoritative.
     */
    input.dpad_mode = PSTVNC_MOUSE_DPAD_POINTER;
    input.dpad_directions = PSTVNC_MOUSE_DIRECTION_RIGHT;
    input.click_buttons = PSTVNC_MOUSE_BUTTON_LCLICK;

    assert(pstvnc_mouse_update(&mouse, &input, &update));
    assert(mouse.click_buttons == PSTVNC_MOUSE_BUTTON_LCLICK);

    memset(&input, 0, sizeof(input));
    input.wheel_click_pressed = 1;

    assert(pstvnc_mouse_update(&mouse, &input, &update));
    assert(mouse.wheel_mode_enabled != 0);

    /*
     * Rebase replaces unpublished local pointer state with the exact state
     * application/main says was successfully published to the remote peer.
     */
    assert(
        pstvnc_mouse_rebase_published_state(
            &mouse,
            100u,
            120u,
            PSTVNC_MOUSE_BUTTON_RCLICK));

    assert(mouse.cursor_x == 100);
    assert(mouse.cursor_y == 120);
    assert(mouse.click_buttons == PSTVNC_MOUSE_BUTTON_RCLICK);

    assert(mouse.wheel_mode_enabled == 0);
    assert(mouse.dpad_hold_direction == 0);
    assert(mouse.dpad_hold_ticks == 0u);
    assert(mouse.dpad_x_q8 == 0);
    assert(mouse.dpad_y_q8 == 0);
    assert(mouse.analog_x_q8 == 0);
    assert(mouse.analog_y_q8 == 0);
    assert(mouse.last_wheel_direction == PSTVNC_MOUSE_WHEEL_NONE);
    assert(mouse.wheel_repeat_countdown == 0u);

    /*
     * Invalid authoritative state must not partially mutate the interpreter.
     */
    assert(
        !pstvnc_mouse_rebase_published_state(
            &mouse,
            640u,
            120u,
            0));

    assert(mouse.cursor_x == 100);
    assert(mouse.cursor_y == 120);
    assert(mouse.click_buttons == PSTVNC_MOUSE_BUTTON_RCLICK);

    assert(
        !pstvnc_mouse_rebase_published_state(
            &mouse,
            100u,
            120u,
            (unsigned char)0x80u));

    assert(mouse.cursor_x == 100);
    assert(mouse.cursor_y == 120);
    assert(mouse.click_buttons == PSTVNC_MOUSE_BUTTON_RCLICK);
}

int main(void)
{
    test_published_state_rebase();
    test_init_and_validation();
    test_dpad_precision_and_direction_change();
    test_dpad_acceleration_and_wheel_boundary();
    test_analog_deadzone_fraction_and_full_speed();
    test_clamping_and_clicks();
    test_dpad_wheel_priority_and_repeat();
    test_wheel_click_toggles_analog_wheel();
    test_dpad_wheel_beats_analog_wheel();
    test_derived_reset_preserves_remote_pointer_state();
    test_transient_history_reset_preserves_wheel_mode();
    test_explicit_derived_reset();

    puts("MOUSE_TEST=PASS");
    return 0;
}

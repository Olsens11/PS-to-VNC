/*
 * File synopsis:
 * Implements PS-to-VNC's pure remote-mouse state and qualified B07
 * pointer/scroll response behavior.
 *
 * The implementation preserves the final B4A response family while separating
 * it from controller acquisition and RFB transport. Input arrives only as
 * already-routed mouse-domain facts, and output is semantic mouse state.
 *
 * Context: docs/CLEAN_ARCHITECTURE.md, "Input/controller"; GitHub Issue #38;
 * docs/audit/B07_B09_INPUT_KEYBOARD_LOCAL_UI.md, B07.
 */

#include "mouse.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

/*
 * Final Test11K-C / Test11K-D analog response constants.
 *
 * The first ~85 percent of stick travel is intentionally shallow. Through
 * approximately half travel the earlier curve is halved, then smoothly rejoins
 * the ~30-percent knee at raw magnitude 108. The final ~15 percent accelerates
 * to full speed.
 */
#define MOUSE_ANALOG_DEADZONE       10
#define MOUSE_ANALOG_HALF_RAW       64
#define MOUSE_ANALOG_KNEE_RAW       108
#define MOUSE_ANALOG_MAX_RAW        127

#define MOUSE_ANALOG_CURVE_ONE      4096u
#define MOUSE_ANALOG_CURVE_KNEE     1229u

/*
 * Final Test11K-G2 progressive D-pad response.
 *
 * Before one second, D-pad motion retains precision behavior. Thereafter it
 * ramps over about 1.5 seconds toward the same six-pixel-per-poll maximum used
 * by a fully deflected analog stick.
 */
#define MOUSE_DPAD_ACCEL_START_TICKS  60u
#define MOUSE_DPAD_ACCEL_RAMP_TICKS   90u

#define MOUSE_DPAD_BASE_VELOCITY_Q8   128
#define MOUSE_DPAD_MAX_VELOCITY_Q8    1536

#define MOUSE_DPAD_WHEEL_REPEAT_POLLS  5u

static void reset_dpad_motion(pstvnc_mouse_t *mouse)
{
    mouse->dpad_hold_direction = 0;
    mouse->dpad_hold_ticks = 0;
    mouse->dpad_x_q8 = 0;
    mouse->dpad_y_q8 = 0;
}

static void reset_analog_motion(pstvnc_mouse_t *mouse)
{
    mouse->analog_x_q8 = 0;
    mouse->analog_y_q8 = 0;
}

static unsigned int analog_speed_curve_q12(int raw_magnitude)
{
    int usable;
    int knee_usable;
    int max_usable;
    unsigned int curve;

    if (raw_magnitude <= MOUSE_ANALOG_DEADZONE)
        return 0;

    if (raw_magnitude > MOUSE_ANALOG_MAX_RAW)
        raw_magnitude = MOUSE_ANALOG_MAX_RAW;

    usable =
        raw_magnitude - MOUSE_ANALOG_DEADZONE;

    knee_usable =
        MOUSE_ANALOG_KNEE_RAW -
        MOUSE_ANALOG_DEADZONE;

    max_usable =
        MOUSE_ANALOG_MAX_RAW -
        MOUSE_ANALOG_DEADZONE;

    if (usable <= knee_usable) {
        unsigned int scale_q12;

        curve =
            (MOUSE_ANALOG_CURVE_KNEE *
             (unsigned int)usable *
             (unsigned int)usable) /
            ((unsigned int)knee_usable *
             (unsigned int)knee_usable);

        if (raw_magnitude <= MOUSE_ANALOG_HALF_RAW) {
            scale_q12 = 2048u;
        } else {
            scale_q12 =
                2048u +
                (2048u *
                 (unsigned int)
                 (raw_magnitude -
                  MOUSE_ANALOG_HALF_RAW)) /
                (unsigned int)
                (MOUSE_ANALOG_KNEE_RAW -
                 MOUSE_ANALOG_HALF_RAW);
        }

        curve =
            (curve * scale_q12) /
            MOUSE_ANALOG_CURVE_ONE;

        if (curve == 0)
            curve = 1;

        return curve;
    }

    return
        MOUSE_ANALOG_CURVE_KNEE +
        ((MOUSE_ANALOG_CURVE_ONE -
          MOUSE_ANALOG_CURVE_KNEE) *
         (unsigned int)(usable - knee_usable)) /
        (unsigned int)(max_usable - knee_usable);
}

static int stick_velocity_q8(unsigned char value)
{
    int displacement = (int)value - 128;
    int sign;
    int raw;
    unsigned int curve;
    int velocity;

    if (displacement < 0) {
        sign = -1;
        raw = -displacement;
    } else {
        sign = 1;
        raw = displacement;
    }

    curve = analog_speed_curve_q12(raw);

    if (curve == 0)
        return 0;

    velocity =
        (1536 * (int)curve) /
        (int)MOUSE_ANALOG_CURVE_ONE;

    /*
     * Q8 value 2 is approximately 0.47 pixel/second at a 60 Hz poll rate.
     * The very small floor is intentional and permits extremely fine motion.
     */
    if (velocity < 2)
        velocity = 2;

    return sign * velocity;
}

static int drain_q8(int *accumulator)
{
    int pixels = 0;

    while (*accumulator >= 256) {
        pixels++;
        *accumulator -= 256;
    }

    while (*accumulator <= -256) {
        pixels--;
        *accumulator += 256;
    }

    return pixels;
}

static int dpad_hold_velocity_q8(unsigned int hold_ticks)
{
    unsigned int elapsed;
    int raw;
    unsigned int curve;

    if (hold_ticks <= MOUSE_DPAD_ACCEL_START_TICKS)
        return MOUSE_DPAD_BASE_VELOCITY_Q8;

    elapsed =
        hold_ticks - MOUSE_DPAD_ACCEL_START_TICKS;

    if (elapsed >= MOUSE_DPAD_ACCEL_RAMP_TICKS)
        return MOUSE_DPAD_MAX_VELOCITY_Q8;

    raw =
        MOUSE_ANALOG_DEADZONE +
        ((MOUSE_ANALOG_MAX_RAW -
          MOUSE_ANALOG_DEADZONE) *
         (int)elapsed) /
        (int)MOUSE_DPAD_ACCEL_RAMP_TICKS;

    curve = analog_speed_curve_q12(raw);

    return
        MOUSE_DPAD_BASE_VELOCITY_Q8 +
        ((MOUSE_DPAD_MAX_VELOCITY_Q8 -
          MOUSE_DPAD_BASE_VELOCITY_Q8) *
         (int)curve) /
        (int)MOUSE_ANALOG_CURVE_ONE;
}

static void apply_dpad_pointer(
    pstvnc_mouse_t *mouse,
    uint16_t directions,
    int *dx,
    int *dy)
{
    int do_step = 0;

    directions &=
        (uint16_t)PSTVNC_MOUSE_DIRECTION_MASK;

    if (directions == 0) {
        reset_dpad_motion(mouse);
        return;
    }

    if (directions != mouse->dpad_hold_direction) {
        /*
         * A fresh direction or any growth/shrink/change of the directional
         * combination starts a new precision gesture.
         */
        mouse->dpad_hold_direction = directions;
        mouse->dpad_hold_ticks = 0;
        mouse->dpad_x_q8 = 0;
        mouse->dpad_y_q8 = 0;
        do_step = 1;
    } else {
        mouse->dpad_hold_ticks++;

        if (mouse->dpad_hold_ticks <
            MOUSE_DPAD_ACCEL_START_TICKS) {

            /*
             * Final 11K-F precision phase: approximately 300 ms delay,
             * then two pixels every four controller polls.
             */
            if (mouse->dpad_hold_ticks >= 18u &&
                ((mouse->dpad_hold_ticks - 18u) % 4u) == 0u)
                do_step = 1;
        } else {
            int velocity =
                dpad_hold_velocity_q8(
                    mouse->dpad_hold_ticks);

            if (directions & PSTVNC_MOUSE_DIRECTION_LEFT)
                mouse->dpad_x_q8 -= velocity;

            if (directions & PSTVNC_MOUSE_DIRECTION_RIGHT)
                mouse->dpad_x_q8 += velocity;

            if (directions & PSTVNC_MOUSE_DIRECTION_UP)
                mouse->dpad_y_q8 -= velocity;

            if (directions & PSTVNC_MOUSE_DIRECTION_DOWN)
                mouse->dpad_y_q8 += velocity;

            *dx += drain_q8(&mouse->dpad_x_q8);
            *dy += drain_q8(&mouse->dpad_y_q8);
        }
    }

    if (!do_step)
        return;

    if (directions & PSTVNC_MOUSE_DIRECTION_LEFT)
        *dx -= 2;

    if (directions & PSTVNC_MOUSE_DIRECTION_RIGHT)
        *dx += 2;

    if (directions & PSTVNC_MOUSE_DIRECTION_UP)
        *dy -= 2;

    if (directions & PSTVNC_MOUSE_DIRECTION_DOWN)
        *dy += 2;
}

static void apply_analog_pointer(
    pstvnc_mouse_t *mouse,
    unsigned char stick_x,
    unsigned char stick_y,
    int *dx,
    int *dy)
{
    int vx = stick_velocity_q8(stick_x);
    int vy = stick_velocity_q8(stick_y);

    /*
     * Returning an axis to center/deadzone immediately discards the partial
     * pixel for that axis. It cannot emerge later after a fresh deflection.
     */
    if (vx == 0)
        mouse->analog_x_q8 = 0;
    else
        mouse->analog_x_q8 += vx;

    if (vy == 0)
        mouse->analog_y_q8 = 0;
    else
        mouse->analog_y_q8 += vy;

    *dx += drain_q8(&mouse->analog_x_q8);
    *dy += drain_q8(&mouse->analog_y_q8);
}

static pstvnc_mouse_wheel_direction_t
wheel_direction_for_dpad(uint16_t directions)
{
    directions &=
        (uint16_t)PSTVNC_MOUSE_DIRECTION_MASK;

    /*
     * Preserve final Triangle+D-pad precedence: a valid vertical direction
     * wins over horizontal when a diagonal combination is present.
     */
    if ((directions & PSTVNC_MOUSE_DIRECTION_UP) &&
        !(directions & PSTVNC_MOUSE_DIRECTION_DOWN))
        return PSTVNC_MOUSE_WHEEL_UP;

    if ((directions & PSTVNC_MOUSE_DIRECTION_DOWN) &&
        !(directions & PSTVNC_MOUSE_DIRECTION_UP))
        return PSTVNC_MOUSE_WHEEL_DOWN;

    if ((directions & PSTVNC_MOUSE_DIRECTION_LEFT) &&
        !(directions & PSTVNC_MOUSE_DIRECTION_RIGHT))
        return PSTVNC_MOUSE_WHEEL_LEFT;

    if ((directions & PSTVNC_MOUSE_DIRECTION_RIGHT) &&
        !(directions & PSTVNC_MOUSE_DIRECTION_LEFT))
        return PSTVNC_MOUSE_WHEEL_RIGHT;

    return PSTVNC_MOUSE_WHEEL_NONE;
}

static pstvnc_mouse_wheel_direction_t
wheel_direction_for_stick(
    unsigned char stick_x,
    unsigned char stick_y,
    int *magnitude)
{
    int x = (int)stick_x - 128;
    int y = (int)stick_y - 128;
    int absolute_x = (x < 0) ? -x : x;
    int absolute_y = (y < 0) ? -y : y;

    *magnitude = 0;

    if (absolute_x <= MOUSE_ANALOG_DEADZONE &&
        absolute_y <= MOUSE_ANALOG_DEADZONE)
        return PSTVNC_MOUSE_WHEEL_NONE;

    /*
     * Dominant-axis scrolling. Equal deflection deliberately resolves
     * vertically, matching the final qualified behavior.
     */
    if (absolute_y >= absolute_x) {
        *magnitude = absolute_y;

        return
            (y < 0)
            ? PSTVNC_MOUSE_WHEEL_UP
            : PSTVNC_MOUSE_WHEEL_DOWN;
    }

    *magnitude = absolute_x;

    return
        (x < 0)
        ? PSTVNC_MOUSE_WHEEL_LEFT
        : PSTVNC_MOUSE_WHEEL_RIGHT;
}

static unsigned int wheel_repeat_delay(
    int magnitude)
{
    unsigned int curve;
    unsigned int clicks_per_second_x10;
    unsigned int interval_polls;

    curve = analog_speed_curve_q12(magnitude);

    if (curve == 0)
        return 59u;

    clicks_per_second_x10 =
        (200u * curve) /
        MOUSE_ANALOG_CURVE_ONE;

    /*
     * Near center permit scrolling as slowly as roughly one notch every
     * two seconds rather than making low deflection disappear.
     */
    if (clicks_per_second_x10 < 5u)
        clicks_per_second_x10 = 5u;

    interval_polls =
        (600u + clicks_per_second_x10 / 2u) /
        clicks_per_second_x10;

    if (interval_polls < 3u)
        interval_polls = 3u;

    return interval_polls - 1u;
}

static void clamp_position(pstvnc_mouse_t *mouse)
{
    if (mouse->cursor_x < 0)
        mouse->cursor_x = 0;

    if (mouse->cursor_x >= (int)mouse->width)
        mouse->cursor_x = (int)mouse->width - 1;

    if (mouse->cursor_y < 0)
        mouse->cursor_y = 0;

    if (mouse->cursor_y >= (int)mouse->height)
        mouse->cursor_y = (int)mouse->height - 1;
}

int pstvnc_mouse_init(
    pstvnc_mouse_t *mouse,
    unsigned int width,
    unsigned int height)
{
    if (mouse == NULL ||
        width == 0 ||
        height == 0 ||
        width > (unsigned int)INT_MAX ||
        height > (unsigned int)INT_MAX)
        return 0;

    memset(mouse, 0, sizeof(*mouse));

    mouse->width = width;
    mouse->height = height;

    /*
     * Preserve the ordinary historical starting position: the logical center
     * of the current remote desktop.
     */
    mouse->cursor_x = (int)(width / 2u);
    mouse->cursor_y = (int)(height / 2u);

    return 1;
}

void pstvnc_mouse_reset_derived(
    pstvnc_mouse_t *mouse)
{
    if (mouse == NULL)
        return;

    reset_dpad_motion(mouse);
    reset_analog_motion(mouse);

    /*
     * A hard mouse-state boundary also leaves analog wheel mode. Cursor
     * position and current remote click-button state remain authoritative,
     * but no pre-boundary mode, fractional motion, or repeat history may
     * resume afterward.
     */
    mouse->wheel_mode_enabled = 0;

    mouse->last_wheel_direction =
        PSTVNC_MOUSE_WHEEL_NONE;

    mouse->wheel_repeat_countdown = 0;
}

int pstvnc_mouse_update(
    pstvnc_mouse_t *mouse,
    const pstvnc_mouse_input_t *input,
    pstvnc_mouse_update_t *update)
{
    int old_x;
    int old_y;
    unsigned char old_click_buttons;
    int dx = 0;
    int dy = 0;

    pstvnc_mouse_wheel_direction_t wheel_direction =
        PSTVNC_MOUSE_WHEEL_NONE;

    unsigned int wheel_repeat_delay_polls = 0;
    int wheel_magnitude = 0;

    if (mouse == NULL ||
        input == NULL ||
        update == NULL ||
        mouse->width == 0 ||
        mouse->height == 0)
        return 0;

    old_x = mouse->cursor_x;
    old_y = mouse->cursor_y;
    old_click_buttons = mouse->click_buttons;

    /*
     * D-pad ownership is explicit. When another input responsibility owns it,
     * no held/fractional D-pad mouse history survives that ownership gap.
     */
    if (input->dpad_mode == PSTVNC_MOUSE_DPAD_POINTER) {
        apply_dpad_pointer(
            mouse,
            input->dpad_directions,
            &dx,
            &dy);

    } else if (input->dpad_mode ==
               PSTVNC_MOUSE_DPAD_WHEEL) {

        reset_dpad_motion(mouse);

        wheel_direction =
            wheel_direction_for_dpad(
                input->dpad_directions);

        if (wheel_direction !=
            PSTVNC_MOUSE_WHEEL_NONE)
            wheel_repeat_delay_polls =
                MOUSE_DPAD_WHEEL_REPEAT_POLLS;

    } else {
        reset_dpad_motion(mouse);
    }

    /*
     * A wheel click toggles analog wheel mode.
     *
     * Treat the click itself as a clean mode boundary: discard fractional
     * cursor motion, restart wheel-repeat history, and deliberately ignore
     * stick displacement for this one sample. Physically clicking L3 can move
     * the stick slightly; that should not bump the cursor or manufacture a
     * wheel notch.
     */
    {
        int wheel_mode_toggled = 0;

        if (input->wheel_click_pressed) {
            mouse->wheel_mode_enabled =
                !mouse->wheel_mode_enabled;

            reset_analog_motion(mouse);

            mouse->last_wheel_direction =
                PSTVNC_MOUSE_WHEEL_NONE;

            mouse->wheel_repeat_countdown = 0;
            wheel_mode_toggled = 1;
        }

        if (!input->stick_available ||
            wheel_mode_toggled) {

            reset_analog_motion(mouse);

        } else if (mouse->wheel_mode_enabled) {

            reset_analog_motion(mouse);

            /*
             * Triangle+D-pad wheel input has deliberate priority. Analog
             * wheel motion fills the wheel slot only when no D-pad wheel
             * direction was selected for this sample.
             */
            if (wheel_direction ==
                PSTVNC_MOUSE_WHEEL_NONE) {

                wheel_direction =
                    wheel_direction_for_stick(
                        input->stick_x,
                        input->stick_y,
                        &wheel_magnitude);

                if (wheel_direction !=
                    PSTVNC_MOUSE_WHEEL_NONE) {

                    wheel_repeat_delay_polls =
                        wheel_repeat_delay(
                            wheel_magnitude);
                }
            }
        } else {
            apply_analog_pointer(
                mouse,
                input->stick_x,
                input->stick_y,
                &dx,
                &dy);
        }
    }

    mouse->cursor_x += dx;
    mouse->cursor_y += dy;

    clamp_position(mouse);

    mouse->click_buttons =
        input->click_buttons &
        (unsigned char)PSTVNC_MOUSE_BUTTON_MASK;

    memset(update, 0, sizeof(*update));

    update->cursor_x = (unsigned int)mouse->cursor_x;
    update->cursor_y = (unsigned int)mouse->cursor_y;
    update->click_buttons = mouse->click_buttons;

    update->pointer_changed =
        mouse->cursor_x != old_x ||
        mouse->cursor_y != old_y ||
        mouse->click_buttons != old_click_buttons;

    /*
     * Wheel output is one semantic notch. Direction changes fire immediately;
     * a held direction repeats after the selected countdown exactly as in the
     * final B4A controller behavior.
     */
    if (wheel_direction !=
        PSTVNC_MOUSE_WHEEL_NONE) {

        if (wheel_direction !=
            mouse->last_wheel_direction)
            mouse->wheel_repeat_countdown = 0;

        if (mouse->wheel_repeat_countdown == 0) {
            update->wheel_direction =
                wheel_direction;

            mouse->wheel_repeat_countdown =
                wheel_repeat_delay_polls;
        } else {
            mouse->wheel_repeat_countdown--;
        }

        mouse->last_wheel_direction =
            wheel_direction;
    } else {
        mouse->last_wheel_direction =
            PSTVNC_MOUSE_WHEEL_NONE;

        mouse->wheel_repeat_countdown = 0;
    }

    return 1;
}

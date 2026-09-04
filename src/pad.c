/*
 * File synopsis:
 * Uses PS2SDK libpad directly to initialize controller RPC ownership, open one
 * explicit pad endpoint, establish DualShock analog mode when supported, and
 * publish factual physical samples plus immediate press/release transitions.
 *
 * This module intentionally stops at physical controller acquisition. Pointer
 * interpretation, chord timing, bindings, UI meaning, RFB serialization, and
 * haptic policy belong to later independent consumers.
 *
 * Context: docs/adr/0002-use-libpad-directly-reference-opl.md and GitHub
 * Issue #38. Open PS2 Loader's pad implementation is the preferred mature
 * usage reference; this source is independently written against libpad rather
 * than copied from OPL.
 */

#include "pad.h"

#include <stddef.h>
#include <string.h>

static int libpad_ready;

static int pad_state_is_readable(int state)
{
    return state == PAD_STATE_STABLE || state == PAD_STATE_FINDCTP1;
}

static int wait_for_pad_settle(pstvnc_pad_t *pad)
{
    int state;

    /*
     * Mode-changing libpad requests temporarily publish EXECCMD while PADMAN
     * performs the operation. The established PS2SDK/OPL pattern is to wait
     * until the endpoint is readable again. DISCONN and ERROR are terminal for
     * this attempt so a disappearing or failed pad cannot trap us here forever.
     */
    for (;;) {
        state = padGetState(pad->port, pad->slot);
        pad->state = state;

        if (pad_state_is_readable(state) ||
            state == PAD_STATE_DISCONN ||
            state == PAD_STATE_ERROR)
            return state;
    }
}

static void invalidate_observation(pstvnc_pad_t *pad)
{
    /*
     * A connection/ownership boundary destroys continuity with the preceding
     * sample. Clearing history prevents a button already held on reacquisition
     * from being manufactured as a fresh press.
     */
    memset(&pad->buttons, 0, sizeof(pad->buttons));
    pad->sample_length = 0;
    pad->buttons_down = 0;
    pad->buttons_pressed = 0;
    pad->buttons_released = 0;
    pad->history_valid = 0;
}

static int configure_controller_mode(pstvnc_pad_t *pad)
{
    int state;
    int modes;
    int i;

    state = wait_for_pad_settle(pad);
    if (state == PAD_STATE_DISCONN ||
        state == PAD_STATE_ERROR)
        return 0;

    if (!pad_state_is_readable(state))
        return -1;

    /*
     * A zero mode-table count is the conventional digital-pad case. Digital
     * controllers remain valid button sources; analog mode is requested only
     * when libpad says the endpoint actually advertises DualShock capability.
     */
    modes = padInfoMode(
        pad->port,
        pad->slot,
        PAD_MODETABLE,
        -1);

    if (modes > 0) {
        for (i = 0; i < modes; i++) {
            if (padInfoMode(
                    pad->port,
                    pad->slot,
                    PAD_MODETABLE,
                    i) == PAD_TYPE_DUALSHOCK) {
                /*
                 * PS-to-VNC needs deterministic analog-stick availability.
                 * Locking DualShock mode prevents the controller's mode button
                 * from silently changing the physical data contract at runtime.
                 */
                if (padSetMainMode(
                        pad->port,
                        pad->slot,
                        PAD_MMODE_DUALSHOCK,
                        PAD_MMODE_LOCK) <= 0)
                    return -1;

                state = wait_for_pad_settle(pad);
                if (state == PAD_STATE_DISCONN ||
                    state == PAD_STATE_ERROR)
                    return 0;

                if (!pad_state_is_readable(state))
                    return -1;

                if (padInfoMode(
                        pad->port,
                        pad->slot,
                        PAD_MODECURID,
                        0) != PAD_TYPE_DUALSHOCK)
                    return -1;

                break;
            }
        }
    }

    pad->connection_configured = 1;
    return 1;
}

int pstvnc_pad_init(void)
{
    /*
     * padInit() is process/libpad-wide rather than per controller endpoint.
     * Keep that lifecycle separate from pstvnc_pad_t so future additional
     * (port, slot) instances do not reinitialize global libpad state.
     */
    if (libpad_ready)
        return 0;

    if (padInit(0) <= 0)
        return -1;

    libpad_ready = 1;
    return 0;
}

void pstvnc_pad_shutdown(void)
{
    if (!libpad_ready)
        return;

    (void)padEnd();
    libpad_ready = 0;
}

int pstvnc_pad_open(
    pstvnc_pad_t *pad,
    int port,
    int slot)
{
    if (pad == NULL || !libpad_ready)
        return -1;

    memset(pad, 0, sizeof(*pad));

    pad->port = port;
    pad->slot = slot;
    pad->state = PAD_STATE_DISCONN;

    if (padPortOpen(
            pad->port,
            pad->slot,
            pad->dma_buffer) == 0)
        return -1;

    pad->opened = 1;
    return 0;
}

int pstvnc_pad_poll(pstvnc_pad_t *pad)
{
    int state;
    int configuration_result;
    struct padButtonStatus sample;
    unsigned char sample_length;
    uint16_t new_buttons_down;
    size_t required_button_bytes;

    if (pad == NULL || !pad->opened)
        return -1;

    /*
     * Edge masks describe only the newest successful observation. Clearing
     * them before every poll prevents one transition from being consumed more
     * than once merely because a subsequent poll had no readable sample.
     */
    pad->buttons_pressed = 0;
    pad->buttons_released = 0;

    state = padGetState(pad->port, pad->slot);
    pad->state = state;

    if (state == PAD_STATE_DISCONN ||
        state == PAD_STATE_FINDPAD ||
        state == PAD_STATE_ERROR) {
        /*
         * None of these states provides a trustworthy physical sample.
         * ERROR is a libpad endpoint state rather than, by itself, proof that
         * the whole PS-to-VNC input subsystem is unrecoverable. Revoke
         * continuity and allow a later readable state to begin a fresh
         * connection epoch.
         */
        pad->connection_configured = 0;
        invalidate_observation(pad);
        return 0;
    }

    if (!pad_state_is_readable(state))
        return 0;

    if (!pad->connection_configured) {
        configuration_result = configure_controller_mode(pad);

        if (configuration_result <= 0) {
            pad->connection_configured = 0;
            invalidate_observation(pad);
            return configuration_result;
        }
    }

    /*
     * libpad copies only the byte count supplied by PADMAN. Read into a fresh
     * temporary so shorter digital-pad packets cannot leave stale analog or
     * pressure bytes from an earlier, longer sample.
     */
    memset(&sample, 0, sizeof(sample));
    sample_length = padRead(
        pad->port,
        pad->slot,
        &sample);

    if (sample_length == 0)
        return 0;

    required_button_bytes =
        offsetof(struct padButtonStatus, btns) +
        sizeof(sample.btns);

    if ((size_t)sample_length < required_button_bytes)
        return 0;

    /*
     * libpad's btns field is active-low. Invert only the standard 16-bit PAD_*
     * mask so downstream code can ask ordinary physical questions while still
     * using the native libpad button constants.
     */
    new_buttons_down =
        (uint16_t)(0xffffu ^ (uint16_t)sample.btns);

    pad->buttons = sample;
    pad->sample_length = sample_length;

    if (pad->history_valid) {
        pad->buttons_pressed =
            (uint16_t)(new_buttons_down & ~pad->buttons_down);
        pad->buttons_released =
            (uint16_t)(pad->buttons_down & ~new_buttons_down);
    } else {
        /*
         * The first sample after open/reacquisition establishes a baseline.
         * Already-held buttons are visible in buttons_down but are not falsely
         * reported as having been pressed inside PS-to-VNC.
         */
        pad->history_valid = 1;
    }

    pad->buttons_down = new_buttons_down;
    return 1;
}

void pstvnc_pad_close(pstvnc_pad_t *pad)
{
    if (pad == NULL || !pad->opened)
        return;

    (void)padPortClose(pad->port, pad->slot);

    pad->opened = 0;
    pad->connection_configured = 0;
    pad->state = PAD_STATE_DISCONN;
    invalidate_observation(pad);
}

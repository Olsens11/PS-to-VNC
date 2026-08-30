 void controller_thread(void *arg)
{
    int sock = *(int *)arg;
    int cursor_x = desktop_width / 2;
    int cursor_y = desktop_height / 2;
    unsigned char last_button_mask = 0;
    unsigned int last_pressed = 0;

    /*
     * Precision D-pad mouse movement:
     * immediate 2px step, then delayed repeat if held.
     */
    unsigned int dpad_hold_direction = 0;
    unsigned int dpad_hold_ticks = 0;

    /*
     * Fractional Q8 movement for accelerated D-pad motion.
     */
    int dpad_x_q8 = 0;
    int dpad_y_q8 = 0;

    /*
     * Fractional Q8 cursor motion.
     *
     * This is what permits genuinely slow movement near stick center:
     * several controller polls may be required to accumulate one pixel.
     */
    int analog_x_q8 = 0;
    int analog_y_q8 = 0;

    /*
     * Scroll repeat state survives from one controller poll to the next.
     */
    unsigned char last_scroll_mask = 0;
    unsigned int scroll_repeat_countdown = 0;
    unsigned int hotkey_observed_mask = 0;
    unsigned int hotkey_stable_polls = 0;
    int hotkey_active_action = RUNTIME_ACTION_NONE;
    int hotkey_active_trigger = 0;
    unsigned int hotkey_hold_polls = 0;
    int hotkey_hold_last_seconds = -1;
    unsigned int hotkey_release_armed_mask = 0;
    int hotkey_release_armed_action = RUNTIME_ACTION_NONE;
    int hotkey_latched = 0;
    int hotkey_desktop_session = 0;

    /*
     * Buttons consumed by a local PS2VNC UI stay quarantined from
     * remote-desktop input until those physical buttons are released.
     *
     * This is stronger than same-poll consumption: a Circle press that
     * closes a menu may still be physically down on the next controller
     * poll, when the desktop branch has become active again.
     */
    unsigned int local_ui_quarantine = 0;

    const struct timespec poll_delay = {
        0,
        16667000
    };

    for (;;) {
        struct padButtonStatus buttons;
        unsigned int pressed;
        unsigned int changed;
        unsigned char button_mask;
        int old_x = cursor_x;
        int old_y = cursor_y;
        int dx = 0;
        int dy = 0;

        unsigned char scroll_mask = 0;
        unsigned int scroll_delay = 0;
        int scroll_magnitude = 0;

        /*
         * G2-C pad ownership handoff.
         *
         * The acknowledgement is published only after this thread has
         * reached a point immediately before its next libpad access.
         * While acknowledged, this thread performs no padGetState/padRead
         * calls and therefore the main thread may temporarily own libpad.
         */
        if (controller_pad_pause_requested) {
            controller_pad_pause_ack = 1;

            while (controller_pad_pause_requested &&
                   !exit_requested) {
                nanosleep(&poll_delay, NULL);
            }

            controller_pad_pause_ack = 0;

            if (exit_requested) {
                ExitThread();
                return;
            }

            /*
             * The main thread will return ownership only after a clean
             * physical release. Forget every pre-handoff edge/hold state so
             * no calibration button can become desktop input afterward.
             */
            last_pressed = 0;
            last_button_mask = 0;

            dpad_hold_direction = 0;
            dpad_hold_ticks = 0;
            dpad_x_q8 = 0;
            dpad_y_q8 = 0;

            analog_x_q8 = 0;
            analog_y_q8 = 0;

            last_scroll_mask = 0;
            scroll_repeat_countdown = 0;

            hotkey_observed_mask = 0;
            hotkey_stable_polls = 0;
            hotkey_active_action = RUNTIME_ACTION_NONE;
            hotkey_active_trigger = 0;
            hotkey_hold_polls = 0;
            hotkey_hold_last_seconds = -1;
            hotkey_release_armed_mask = 0;
            hotkey_release_armed_action = RUNTIME_ACTION_NONE;
            hotkey_latched = 0;
            hotkey_desktop_session = 0;

            local_ui_quarantine = 0;

            continue;
        }

        controller_loop_counter++;

        if (ps2vnc_system_refresh_update_state() &&
            system_menu_visible) {

            (void)wake_renderer_with_pointer_jiggle(
                sock, cursor_x, cursor_y);
        }

        if (padGetState(0, 0) == PAD_STATE_STABLE &&
            padRead(0, 0, &buttons) != 0) {

            pressed = 0xFFFFu ^ (unsigned int)buttons.btns;
            changed = pressed ^ last_pressed;

            /*
             * Release is the only thing that clears local ownership.
             * Held buttons therefore cannot leak into the remote desktop
             * after a local menu closes.
             */
            local_ui_quarantine &= pressed;

            /*
             * TEST15E3-G generic runtime-hotkey arbitration.
             *
             * Only buttons participating in runtime bindings are sampled.
             * The exact held mask must remain unchanged for a short settle
             * interval before a multi-button binding owns the session.
             *
             * Once an action actually fires, the hotkey session remains
             * latched until all hotkey buttons are released.  Therefore a
             * shrinking chord cannot accidentally fire one of its subsets.
             *
             * Desktop-only bindings never delay OSK or local-menu controls.
             * Global Refresh/Exit bindings remain observable in every context.
             */
            {
                unsigned int hk =
                    pressed & RUNTIME_HOTKEY_BUTTON_MASK;

                int desktop_hotkey_context =
                    !system_menu_visible &&
                    !display_menu_visible &&
                    !osk_visible;

                if (hotkey_latched) {
                    if (hk == 0) {
                        hotkey_latched = 0;
                        hotkey_observed_mask = 0;
                        hotkey_stable_polls = 0;
                        hotkey_active_action = RUNTIME_ACTION_NONE;
                        hotkey_active_trigger = 0;
                        hotkey_hold_polls = 0;
                        hotkey_hold_last_seconds = -1;
                        hotkey_release_armed_mask = 0;
                        hotkey_release_armed_action =
                            RUNTIME_ACTION_NONE;
                        hotkey_desktop_session = 0;
                        debug_hotkey_settle_target = 0;
                    }

                } else if (hk != hotkey_observed_mask) {
                    unsigned int old_mask = hotkey_observed_mask;
                    const ps2vnc_hotkey_binding_t *new_release = NULL;
                    int release_action = RUNTIME_ACTION_NONE;
                    int pure_growth =
                        ((old_mask & hk) == old_mask);
                    int pure_shrink =
                        ((hk & old_mask) == hk);

                    /*
                     * Record where a new hotkey session began.  A button
                     * pressed while the OSK/menu owns input must never turn
                     * into a desktop shortcut merely because that context
                     * changes before the button is released.
                     */
                    if (hotkey_observed_mask == 0 && hk != 0)
                        hotkey_desktop_session =
                            desktop_hotkey_context;

                    /*
                     * An armed RELEASE binding fires only when its exact mask
                     * is broken by releasing buttons. Growing the gesture
                     * cancels it instead, preventing subset-action leakage.
                     */
                    if (hotkey_release_armed_action !=
                            RUNTIME_ACTION_NONE) {
                        if (hotkey_release_armed_mask == old_mask &&
                            pure_shrink &&
                            desktop_hotkey_context &&
                            hotkey_desktop_session) {
                            release_action =
                                hotkey_release_armed_action;
                        }

                        hotkey_release_armed_mask = 0;
                        hotkey_release_armed_action =
                            RUNTIME_ACTION_NONE;
                    }

                    hotkey_observed_mask = hk;
                    hotkey_stable_polls = 0;
                    hotkey_active_action = RUNTIME_ACTION_NONE;
                    hotkey_active_trigger = 0;
                    hotkey_hold_polls = 0;
                    hotkey_hold_last_seconds = -1;
                    debug_hotkey_settle_target = 0;

                    /*
                     * Changing a pending hold chord cancels its countdown,
                     * but deliberately leaves the System menu visible.
                     */
                    if (system_menu_countdown_action !=
                            RUNTIME_ACTION_NONE) {
                        system_menu_countdown_action =
                            RUNTIME_ACTION_NONE;
                        system_menu_countdown_seconds = 0;
                        osk_generation++;
                    }

                    /*
                     * Arm RELEASE only when arriving at its exact mask by
                     * adding buttons. Shrinking from a larger chord cannot
                     * manufacture a release shortcut.
                     */
                    if (pure_growth && hk != 0) {
                        new_release =
                            runtime_hotkey_lookup(
                                hk,
                                desktop_hotkey_context &&
                                hotkey_desktop_session);

                        if (new_release != NULL &&
                            new_release->trigger ==
                                HOTKEY_TRIGGER_RELEASE) {
                            hotkey_release_armed_mask = hk;
                            hotkey_release_armed_action =
                                new_release->action;
                        }
                    }

                    if (release_action != RUNTIME_ACTION_NONE) {
                        unsigned int keysym = 0;

                        if (release_action ==
                                RUNTIME_ACTION_SEND_TAB)
                            keysym = XK_TAB;
                        else if (release_action ==
                                 RUNTIME_ACTION_SEND_ENTER)
                            keysym = XK_RETURN;

                        if (keysym != 0 &&
                            send_key_tap(sock, keysym) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }

                        if (keysym != 0)
                            (void)wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y);
                    }

                    if (hk == 0)
                        hotkey_desktop_session = 0;

                } else if (hk != 0) {
                    const ps2vnc_hotkey_binding_t *binding;
                    unsigned int settle_polls =
                        RUNTIME_HOTKEY_SETTLE_POLLS;

                    hotkey_stable_polls++;

                    binding =
                        runtime_hotkey_lookup(
                            hk,
                            desktop_hotkey_context &&
                            hotkey_desktop_session);

                    /*
                     * Only ambiguous settle-actions get the longer collection
                     * window.  Maximal chords and HOLD bindings retain the
                     * normal short settle.
                     */
                    if (binding != NULL &&
                        binding->trigger == HOTKEY_TRIGGER_SETTLE &&
                        runtime_hotkey_has_hold_extension(
                            hk,
                            desktop_hotkey_context &&
                            hotkey_desktop_session)) {

                        settle_polls =
                            RUNTIME_HOTKEY_EXTENDABLE_SETTLE_POLLS;
                    }

                    debug_hotkey_settle_target = settle_polls;

                    if (hotkey_active_action ==
                            RUNTIME_ACTION_NONE &&
                        binding != NULL &&
                        binding->trigger != HOTKEY_TRIGGER_RELEASE &&
                        hotkey_stable_polls >= settle_polls) {

                        hotkey_active_action = binding->action;
                        hotkey_active_trigger = binding->trigger;

                        if (binding->trigger ==
                                HOTKEY_TRIGGER_SETTLE) {

                            if (binding->action ==
                                    RUNTIME_ACTION_SEND_ENTER) {

                                if (send_key_tap(
                                        sock, XK_RETURN) < 0) {
                                    controller_thread_error = 1;
                                    ExitThread();
                                }

                                (void)wake_renderer_with_pointer_jiggle(
                                    sock, cursor_x, cursor_y);

                            } else if (binding->action ==
                                       RUNTIME_ACTION_DISPLAY_SETTINGS) {

                                system_menu_visible = 0;
                                system_menu_status =
                                    SYSTEM_MENU_STATUS_NONE;

                                display_menu_open_for_active(0);

                                if (last_button_mask != 0) {
                                    (void)send_pointer_event(
                                        sock, 0,
                                        (unsigned int)cursor_x,
                                        (unsigned int)cursor_y);
                                    last_button_mask = 0;
                                }

                                (void)wake_renderer_with_pointer_jiggle(
                                    sock, cursor_x, cursor_y);
                            }

                            hotkey_latched = 1;

                        } else if (binding->trigger ==
                                   HOTKEY_TRIGGER_HOLD) {

                            if (!ps2vnc_system_overlay_allowed()) {

                                hotkey_active_action =
                                    RUNTIME_ACTION_NONE;
                                hotkey_active_trigger = 0;
                                hotkey_latched = 1;

                            } else {

                                /*
                                 * GLOBAL OVERLAY:
                                 *
                                 * Do not clear Display Settings or the OSK.
                                 * Existing renderer/input priority makes
                                 * System temporarily own the foreground while
                                 * preserving the ordinary UI underneath.
                                 */
                                system_menu_visible = 1;

                                system_menu_status =
                                    SYSTEM_MENU_STATUS_NONE;

                                system_menu_selection =
                                    (binding->action ==
                                     RUNTIME_ACTION_EXIT_TO_SYSTEM_MENU)
                                    ? SYSTEM_MENU_EXIT
                                    : SYSTEM_MENU_REFRESH;

                                system_menu_countdown_action =
                                    binding->action;
                                system_menu_countdown_seconds = 2;
                                hotkey_hold_polls = 0;
                                hotkey_hold_last_seconds = -1;
                                osk_generation++;

                                (void)wake_renderer_with_pointer_jiggle(
                                    sock, cursor_x, cursor_y);
                            }
                        }
                    }

                    if (hotkey_active_trigger ==
                            HOTKEY_TRIGGER_HOLD) {
                        unsigned int remaining;

                        hotkey_hold_polls++;

                        remaining =
                            (SYSTEM_CHORD_HOLD_POLLS >
                             hotkey_hold_polls)
                            ? ((SYSTEM_CHORD_HOLD_POLLS -
                                hotkey_hold_polls + 59u) / 60u)
                            : 0u;

                        if ((int)remaining !=
                                hotkey_hold_last_seconds) {
                            hotkey_hold_last_seconds = (int)remaining;
                            system_menu_countdown_seconds = remaining;
                            osk_generation++;
                        }

                        if (hotkey_hold_polls >=
                                SYSTEM_CHORD_HOLD_POLLS) {

                            system_menu_countdown_action =
                                RUNTIME_ACTION_NONE;
                            system_menu_countdown_seconds = 0;
                            osk_generation++;
                            hotkey_latched = 1;

                            if (hotkey_active_action ==
                                    RUNTIME_ACTION_REFRESH_RFB) {

                                (void)
                                    ps2vnc_system_refresh_accept_request();

                            } else if (hotkey_active_action ==
                                       RUNTIME_ACTION_EXIT_TO_SYSTEM_MENU) {

                                if (ps2vnc_system_exit_accept_request()) {

                                    debug_hotkey_mask =
                                        hotkey_observed_mask;
                                    debug_hotkey_stable_polls =
                                        hotkey_stable_polls;
                                    debug_hotkey_action =
                                        hotkey_active_action;
                                    debug_hotkey_trigger =
                                        hotkey_active_trigger;
                                    debug_hotkey_hold_polls =
                                        hotkey_hold_polls;
                                    debug_hotkey_latched =
                                        hotkey_latched;
                                    debug_hotkey_quarantine =
                                        local_ui_quarantine;

                                    ExitThread();
                                    return;
                                }
                            }
                        }
                    }
                }
            }

            debug_hotkey_mask = hotkey_observed_mask;
            debug_hotkey_stable_polls = hotkey_stable_polls;
            debug_hotkey_action = hotkey_active_action;
            debug_hotkey_trigger = hotkey_active_trigger;
            debug_hotkey_hold_polls = hotkey_hold_polls;
            debug_hotkey_latched = hotkey_latched;
            debug_hotkey_quarantine = local_ui_quarantine;

            /*
             * Display confirmation owns a real wall-clock deadline.
             * It starts before the hazardous switch; while the candidate
             * switch is still incomplete the UI remains hidden, but timeout
             * can already publish ROLLBACK and interrupt recv_exact().
             */
            if (display_confirm_pending &&
                display_confirm_action == DISPLAY_CONFIRM_ACTION_NONE &&
                (remote_display_state != REMOTE_DISPLAY_ACTIVE_TEMP ||
                 display_transition_switch_in_progress)) {

                unsigned int remaining =
                    display_confirm_remaining_seconds();

                if (remaining != display_confirm_seconds) {
                    display_confirm_seconds = remaining;

                    if (display_confirm_visible) {
                        osk_generation++;
                        (void)wake_renderer_with_pointer_jiggle(
                            sock, cursor_x, cursor_y);
                    }
                }

                if (remaining == 0 &&
                    display_confirm_pending &&
                    display_confirm_action ==
                        DISPLAY_CONFIRM_ACTION_NONE) {

                    display_confirm_action =
                        DISPLAY_CONFIRM_ACTION_ROLLBACK;
                    display_confirm_visible = 0;
                    osk_generation++;

                    /*
                     * H4C3D16 hard-stall fail-safe.
                     *
                     * Normally the main thread consumes ROLLBACK and restores
                     * A in-process.  If it is still trapped inside the risky
                     * synchronous mode reconstruction, it cannot consume that
                     * request.  Do not touch GS or RFB from this controller
                     * thread; replace the wedged process through the same
                     * established OSDSYS path used by main().
                     *
                     * The durable transaction intentionally remains
                     * provisional.  Existing startup reconciliation therefore
                     * makes the preserved previous profile authoritative.
                     */
                    if (display_transition_switch_in_progress)
                        LoadExecPS2("rom0:OSDSYS", 0, NULL);
                }
            }

            if (display_restore_ack_visible) {
                int ack_changed = 0;

                /*
                 * The restored acknowledgement owns these local UI buttons.
                 * Nothing may leak through to VNC or the menus underneath.
                 */
                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT |
                     PAD_CIRCLE | PAD_CROSS);

                if (!display_restore_ack_request &&
                    (changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {

                    display_restore_ack_request = 1;
                    ack_changed = 1;
                }

                if (ack_changed) {
                    (void)wake_renderer_with_pointer_jiggle(
                        sock, cursor_x, cursor_y);
                }

            } else /*
         * D17AL-F8J2-B4A:
         * Remote transitions now take true libpad ownership at the
         * transition boundary. Do not quarantine ordinary desktop
         * controller input for the complete ACTIVE_TEMP dwell.
         */
        if (display_confirm_pending) {
                int menu_changed = 0;

                /*
                 * F8J2-B3C:
                 *
                 * Remote provisional display control owns the controller
                 * through the SAME decision-tree boundary as the proven local
                 * display-confirmation transaction.
                 *
                 * A non-IDLE remote transaction therefore cannot fall through
                 * to desktop pointer/key/scroll generation.  Physical pad
                 * polling and the existing local/global hotkey machinery stay
                 * alive exactly as they do for the local confirmation UI.
                 *
                 * Human KEEP / GO BACK actions remain local-only below.
                 */

                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT |
                     PAD_CIRCLE | PAD_CROSS);

                if (remote_display_state == REMOTE_DISPLAY_IDLE &&
                    display_confirm_action ==
                        DISPLAY_CONFIRM_ACTION_NONE &&
                    display_confirm_visible &&
                    (((changed & PAD_UP) && (pressed & PAD_UP)) ||
                    ((changed & PAD_DOWN) && (pressed & PAD_DOWN)) ||
                    ((changed & PAD_LEFT) && (pressed & PAD_LEFT)) ||
                    ((changed & PAD_RIGHT) && (pressed & PAD_RIGHT)))) {

                    display_confirm_selection =
                        (display_confirm_selection ==
                         DISPLAY_CONFIRM_GO_BACK)
                        ? DISPLAY_CONFIRM_KEEP
                        : DISPLAY_CONFIRM_GO_BACK;

                    osk_generation++;
                    menu_changed = 1;
                }

                if (remote_display_state == REMOTE_DISPLAY_IDLE &&
                    display_confirm_action ==
                        DISPLAY_CONFIRM_ACTION_NONE &&
                    (changed & PAD_CIRCLE) &&
                    (pressed & PAD_CIRCLE)) {

                    display_confirm_action =
                        DISPLAY_CONFIRM_ACTION_ROLLBACK;
                    display_confirm_visible = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if (remote_display_state == REMOTE_DISPLAY_IDLE &&
                    display_confirm_action ==
                        DISPLAY_CONFIRM_ACTION_NONE &&
                    (changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {

                    display_confirm_action =
                        (display_confirm_selection ==
                         DISPLAY_CONFIRM_KEEP)
                        ? DISPLAY_CONFIRM_ACTION_KEEP
                        : DISPLAY_CONFIRM_ACTION_ROLLBACK;

                    display_confirm_visible = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if (menu_changed) {
                    (void)wake_renderer_with_pointer_jiggle(
                        sock, cursor_x, cursor_y);
                }

            } else if (system_menu_visible) {
                int menu_changed = 0;

                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT |
                     PAD_CIRCLE | PAD_CROSS);

                if (((changed & PAD_UP) && (pressed & PAD_UP)) ||
                    ((changed & PAD_LEFT) && (pressed & PAD_LEFT))) {

                    system_menu_selection =
                        (system_menu_selection +
                         SYSTEM_MENU_ITEM_COUNT - 1) %
                        SYSTEM_MENU_ITEM_COUNT;

                    system_menu_status =
                        SYSTEM_MENU_STATUS_NONE;
                    system_menu_countdown_action =
                        RUNTIME_ACTION_NONE;
                    system_menu_countdown_seconds = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if (((changed & PAD_DOWN) && (pressed & PAD_DOWN)) ||
                    ((changed & PAD_RIGHT) && (pressed & PAD_RIGHT))) {

                    system_menu_selection =
                        (system_menu_selection + 1) %
                        SYSTEM_MENU_ITEM_COUNT;

                    system_menu_status =
                        SYSTEM_MENU_STATUS_NONE;
                    system_menu_countdown_action =
                        RUNTIME_ACTION_NONE;
                    system_menu_countdown_seconds = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_CIRCLE) &&
                    (pressed & PAD_CIRCLE)) {

                    system_menu_visible = 0;
                    system_menu_status =
                        SYSTEM_MENU_STATUS_NONE;
                    system_menu_countdown_action =
                        RUNTIME_ACTION_NONE;
                    system_menu_countdown_seconds = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {

                    system_menu_countdown_action =
                        RUNTIME_ACTION_NONE;
                    system_menu_countdown_seconds = 0;

                    if (system_menu_selection ==
                        SYSTEM_MENU_REFRESH) {

                        (void)
                            ps2vnc_system_refresh_accept_request();

                    } else if (system_menu_selection ==
                               SYSTEM_MENU_DISPLAY) {

                        system_menu_visible = 0;
                        system_menu_status =
                            SYSTEM_MENU_STATUS_NONE;

                        if (display_menu_visible) {

                            /*
                             * Display Settings was already the preserved
                             * underlay. Reveal it exactly where it was.
                             */
                            osk_generation++;
                            menu_changed = 1;

                        } else {

                            /*
                             * Deliberate navigation from desktop/OSK into
                             * Display Settings. The previous OSK is not part
                             * of this new navigation branch.
                             */
                            osk_visible = 0;
                            display_menu_open_for_active(1);
                            menu_changed = 1;
                        }

                    } else {

                        if (ps2vnc_system_exit_accept_request()) {
                            ExitThread();
                            return;
                        }

                        osk_generation++;
                        menu_changed = 1;
                    }
                }

                if (menu_changed) {
                    /*
                     * Local overlay wake is best-effort. Never sacrifice the
                     * controller thread merely because the VNC stream is bad.
                     */
                    (void)wake_renderer_with_pointer_jiggle(
                        sock, cursor_x, cursor_y);
                }


            } else if (display_menu_visible) {

                int menu_changed = 0;

                /*
                 * Square is now owned locally while Display Settings is open.
                 * Outside this menu it remains the desktop Backspace shortcut.
                 */
                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_UP | PAD_DOWN |
                     PAD_LEFT | PAD_RIGHT |
                     PAD_CIRCLE | PAD_CROSS |
                     PAD_SQUARE);

                if ((changed & PAD_UP) &&
                    (pressed & PAD_UP)) {

                    display_menu_selection =
                        display_menu_move_vertical_selection(
                            display_menu_selection,
                            -1);

                    display_menu_error = 0;
                    display_policy_error = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_DOWN) &&
                    (pressed & PAD_DOWN)) {

                    display_menu_selection =
                        display_menu_move_vertical_selection(
                            display_menu_selection,
                            1);

                    display_menu_error = 0;
                    display_policy_error = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_LEFT) &&
                    (pressed & PAD_LEFT)) {

                    display_menu_selection =
                        display_menu_move_horizontal_selection(
                            display_menu_selection,
                            -1);

                    display_menu_error = 0;
                    display_policy_error = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_RIGHT) &&
                    (pressed & PAD_RIGHT)) {

                    display_menu_selection =
                        display_menu_move_horizontal_selection(
                            display_menu_selection,
                            1);

                    display_menu_error = 0;
                    display_policy_error = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_CIRCLE) &&
                    (pressed & PAD_CIRCLE)) {

                    display_menu_visible = 0;
                    display_menu_error = 0;
                    display_policy_error = 0;

                    if (display_menu_parent_system) {
                        display_menu_parent_system = 0;
                        system_menu_selection =
                            SYSTEM_MENU_DISPLAY;
                        system_menu_visible = 1;
                    }

                    osk_generation++;
                    menu_changed = 1;
                }

                /*
                 * Square toggles per-mode admission policy.
                 *
                 * The current active mode is deliberately not lockable from
                 * the UI. Unlocking does not switch into the mode.
                 */
                if ((changed & PAD_SQUARE) &&
                    (pressed & PAD_SQUARE) &&
                    display_menu_tab !=
                        DISPLAY_TAB_ADVANCED) {

                    const ps2vnc_video_mode_t *selected_mode =
                        display_menu_mode_at_selection(
                            display_menu_selection);

                    if (selected_mode != NULL &&
                        selected_mode != active_video_mode &&
                        display_policy_loaded &&
                        display_policy_request ==
                            DISPLAY_POLICY_REQUEST_NONE) {

                        display_policy_error = 0;

                        display_policy_request_selection =
                            display_menu_selection;

                        display_policy_request_value =
                            display_mode_locked[
                                display_menu_selection]
                                ? 0
                                : 1;

                        display_policy_request =
                            DISPLAY_POLICY_REQUEST_LOCK;

                        osk_generation++;
                        menu_changed = 1;
                    }
                }

                if ((changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {

                    /*
                     * ADVANCED currently contains one setting row.
                     */
                    if (display_menu_tab ==
                            DISPLAY_TAB_ADVANCED) {

                        if (display_policy_loaded &&
                            display_policy_request ==
                                DISPLAY_POLICY_REQUEST_NONE) {

                            display_policy_error = 0;

                            display_policy_request_value =
                                display_hide_locked_modes
                                    ? 0
                                    : 1;

                            display_policy_request =
                                DISPLAY_POLICY_REQUEST_HIDE_LOCKED;

                            osk_generation++;
                            menu_changed = 1;
                        }

                    } else {

                        const ps2vnc_video_mode_t *selected_mode =
                            display_menu_mode_at_selection(
                                display_menu_selection);

                        if (selected_mode == NULL) {
                            /*
                             * Empty filtered tab: hard no-op.
                             */

                        } else if (selected_mode ==
                                      active_video_mode) {

                            display_menu_visible = 0;
                            display_menu_error = 0;
                            display_policy_error = 0;
                            display_menu_parent_system = 0;

                            display_calibration_request =
                                display_mode_request_for_selection(
                                    display_menu_selection);

                            osk_generation++;
                            menu_changed = 1;

                        } else if (!display_policy_loaded) {

                            /*
                             * Never switch into an inactive mode without
                             * knowing authoritative lock policy.
                             */
                            display_policy_error = 1;
                            osk_generation++;
                            menu_changed = 1;

                        } else if (display_mode_locked[
                                      display_menu_selection]) {

                            /*
                             * HARD NO-OP:
                             *
                             * No transaction request, no management resize,
                             * no GS operation, no startup persistence.
                             */
                            osk_generation++;
                            menu_changed = 1;

                        } else {

                            display_menu_visible = 0;
                            display_menu_error = 0;
                            display_policy_error = 0;
                            display_menu_parent_system = 0;

                            display_mode_request_remote = 0;
                            display_mode_request =
                                display_mode_request_for_selection(
                                    display_menu_selection);

                            osk_generation++;
                            menu_changed = 1;
                        }
                    }
                }

                if (menu_changed) {
                    if (wake_renderer_with_pointer_jiggle(
                            sock,
                            cursor_x,
                            cursor_y) < 0) {

                        controller_thread_error = 1;
                        ExitThread();
                    }
                }

            } else if (!osk_visible) {

                unsigned int desktop_pressed =
                    pressed & ~local_ui_quarantine;
                unsigned int desktop_changed =
                    desktop_pressed ^
                    (last_pressed & ~local_ui_quarantine);

                /*
                 * Open the OSK with Select.  Reset to Q for predictable
                 * entry and ensure no mouse button remains logically held.
                 */
                if ((desktop_changed & PAD_SELECT) &&
                    (desktop_pressed & PAD_SELECT)) {

                    osk_visible = 1;
                    osk_page = OSK_PAGE_ABC;
                    osk_row = 1;
                    osk_col = 0;
                    osk_shift = 0;
                    osk_ctrl = 0;
                    osk_alt = 0;
                    osk_generation++;

                    if (last_button_mask != 0) {
                        if (send_pointer_event(
                                sock, 0,
                                (unsigned int)cursor_x,
                                (unsigned int)cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        last_button_mask = 0;
                    }

                    if (wake_renderer_with_pointer_jiggle(
                            sock, cursor_x, cursor_y) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }

                    /*
                     * last_pressed always tracks the raw physical pad state.
                     * Quarantine affects consumers, never edge bookkeeping.
                     */
                    last_pressed = pressed;
                    nanosleep(&poll_delay, NULL);
                    continue;
                }

                /*
                 * Quick keyboard shortcuts remain available while using
                 * normal mouse mode.
                 */
                if ((desktop_changed & PAD_SQUARE) &&
                    (desktop_pressed & PAD_SQUARE)) {
                    if (send_key_tap(sock, XK_BACKSPACE) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    if (wake_renderer_with_pointer_jiggle(
                            sock, cursor_x, cursor_y) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                }

                /*
                 * R1=Tab and L1+Start=Enter are resolved by the generic
                 * runtime-hotkey arbiter above.
                 */

                /*
                 * TEST13I:
                 * Triangle + D-pad = mouse-wheel scrolling.
                 *
                 * This takes priority over L1+D-pad and ordinary D-pad
                 * precision mouse movement. The OSK has its own input branch,
                 * so Triangle continues to mean Shift while the OSK is open.
                 *
                 * Vertical directions take priority over horizontal directions
                 * if a diagonal D-pad combination is reported.
                 */
                if (desktop_pressed & PAD_TRIANGLE) {
                    unsigned int dpad =
                        desktop_pressed &
                        (PAD_LEFT | PAD_RIGHT |
                         PAD_UP | PAD_DOWN);

                    /*
                     * Do not preserve any fractional/hold state from ordinary
                     * D-pad mouse movement while Triangle owns the D-pad.
                     */
                    dpad_hold_direction = 0;
                    dpad_hold_ticks = 0;
                    dpad_x_q8 = 0;
                    dpad_y_q8 = 0;

                    if ((dpad & PAD_UP) &&
                        !(dpad & PAD_DOWN)) {
                        scroll_mask = RFB_SCROLL_UP;
                    } else if ((dpad & PAD_DOWN) &&
                               !(dpad & PAD_UP)) {
                        scroll_mask = RFB_SCROLL_DOWN;
                    } else if ((dpad & PAD_LEFT) &&
                               !(dpad & PAD_RIGHT)) {
                        scroll_mask = RFB_SCROLL_LEFT;
                    } else if ((dpad & PAD_RIGHT) &&
                               !(dpad & PAD_LEFT)) {
                        scroll_mask = RFB_SCROLL_RIGHT;
                    }

                    if (scroll_mask != 0)
                        scroll_delay = DPAD_SCROLL_REPEAT_POLLS;

                /*
                 * Test11F: L1 is a physical chord modifier, not a mode.
                 * No state is remembered and nothing can remain toggled.
                 */
                } else if (desktop_pressed & PAD_L1) {
                    if ((desktop_pressed & PAD_UP) &&
                        (desktop_changed & (PAD_L1 | PAD_UP))) {
                        if (send_key_tap(sock, XK_UP) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        if (wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                    }

                    if ((desktop_pressed & PAD_DOWN) &&
                        (desktop_changed & (PAD_L1 | PAD_DOWN))) {
                        if (send_key_tap(sock, XK_DOWN) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        if (wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                    }

                    if ((desktop_pressed & PAD_LEFT) &&
                        (desktop_changed & (PAD_L1 | PAD_LEFT))) {
                        if (send_key_tap(sock, XK_LEFT) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        if (wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                    }

                    if ((desktop_pressed & PAD_RIGHT) &&
                        (desktop_changed & (PAD_L1 | PAD_RIGHT))) {
                        if (send_key_tap(sock, XK_RIGHT) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        if (wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                    }
                } else {
                    /* Normal D-pad = precise mouse movement. */
                    {
                        unsigned int dpad =
                            desktop_pressed &
                            (PAD_LEFT | PAD_RIGHT |
                             PAD_UP | PAD_DOWN);

                        unsigned int dpad_new =
                            desktop_changed &
                            desktop_pressed &
                            (PAD_LEFT | PAD_RIGHT |
                             PAD_UP | PAD_DOWN);

                        int do_dpad_step = 0;

                        if (dpad_new != 0) {
                            /*
                             * Fresh tap: exactly two pixels.
                             */
                            dpad_hold_direction = dpad;
                            dpad_hold_ticks = 0;
                            dpad_x_q8 = 0;
                            dpad_y_q8 = 0;
                            do_dpad_step = 1;

                        } else if (dpad == 0) {
                            dpad_hold_direction = 0;
                            dpad_hold_ticks = 0;
                            dpad_x_q8 = 0;
                            dpad_y_q8 = 0;

                        } else if (dpad != dpad_hold_direction) {
                            /*
                             * Direction change behaves like a fresh tap.
                             */
                            dpad_hold_direction = dpad;
                            dpad_hold_ticks = 0;
                            dpad_x_q8 = 0;
                            dpad_y_q8 = 0;
                            do_dpad_step = 1;

                        } else {
                            dpad_hold_ticks++;

                            if (dpad_hold_ticks <
                                DPAD_ACCEL_START_TICKS) {

                                /*
                                 * Preserve 11K-F exactly during the
                                 * precision phase:
                                 *
                                 * ~300 ms delay, then 2 pixels every
                                 * four controller polls.
                                 */
                                if (dpad_hold_ticks >= 18 &&
                                    ((dpad_hold_ticks - 18) % 4) == 0)
                                    do_dpad_step = 1;

                            } else {
                                int velocity =
                                    dpad_hold_velocity_q8(
                                        dpad_hold_ticks);

                                /*
                                 * After one second, use fractional
                                 * continuous movement so velocity can
                                 * accelerate smoothly.
                                 */
                                if (dpad & PAD_LEFT)
                                    dpad_x_q8 -= velocity;

                                if (dpad & PAD_RIGHT)
                                    dpad_x_q8 += velocity;

                                if (dpad & PAD_UP)
                                    dpad_y_q8 -= velocity;

                                if (dpad & PAD_DOWN)
                                    dpad_y_q8 += velocity;

                                while (dpad_x_q8 >= 256) {
                                    dx++;
                                    dpad_x_q8 -= 256;
                                }

                                while (dpad_x_q8 <= -256) {
                                    dx--;
                                    dpad_x_q8 += 256;
                                }

                                while (dpad_y_q8 >= 256) {
                                    dy++;
                                    dpad_y_q8 -= 256;
                                }

                                while (dpad_y_q8 <= -256) {
                                    dy--;
                                    dpad_y_q8 += 256;
                                }
                            }
                        }

                        if (do_dpad_step) {
                            if (dpad & PAD_LEFT)  dx -= 2;
                            if (dpad & PAD_RIGHT) dx += 2;
                            if (dpad & PAD_UP)    dy -= 2;
                            if (dpad & PAD_DOWN)  dy += 2;
                        }
                    }
                }

                /*
                 * Left analog normally remains accelerated mouse movement.
                 *
                 * While L3 is held, however, the same stick becomes an
                 * analog mouse wheel and does NOT move the cursor.
                 */
                if ((buttons.mode & 0xF0) == 0x70) {
                    if (desktop_pressed & PAD_L3) {
                        /*
                         * L3 is a hard pointer freeze.
                         *
                         * Clear fractional mouse motion as soon as scrolling
                         * begins so releasing L3 cannot reveal an old partial
                         * cursor movement.
                         */
                        analog_x_q8 = 0;
                        analog_y_q8 = 0;

                        /*
                         * Triangle+D-pad has explicit priority. Only derive
                         * scrolling from the analog stick if no D-pad wheel
                         * direction was selected above.
                         */
                        if (scroll_mask == 0) {
                            scroll_mask = scroll_button_for_stick(
                                buttons.ljoy_h,
                                buttons.ljoy_v,
                                &scroll_magnitude
                            );

                            if (scroll_mask != 0) {
                                scroll_delay =
                                    scroll_repeat_delay(scroll_magnitude);
                            }
                        }
                    } else {
                        int vx =
                            stick_velocity_q8(buttons.ljoy_h);

                        int vy =
                            stick_velocity_q8(buttons.ljoy_v);

                        /*
                         * Returning an axis to center immediately clears its
                         * fractional remainder.
                         */
                        if (vx == 0)
                            analog_x_q8 = 0;
                        else
                            analog_x_q8 += vx;

                        if (vy == 0)
                            analog_y_q8 = 0;
                        else
                            analog_y_q8 += vy;

                        while (analog_x_q8 >= 256) {
                            dx++;
                            analog_x_q8 -= 256;
                        }

                        while (analog_x_q8 <= -256) {
                            dx--;
                            analog_x_q8 += 256;
                        }

                        while (analog_y_q8 >= 256) {
                            dy++;
                            analog_y_q8 -= 256;
                        }

                        while (analog_y_q8 <= -256) {
                            dy--;
                            analog_y_q8 += 256;
                        }
                    }
                }

                cursor_x += dx;
                cursor_y += dy;

                if (cursor_x < 0)
                    cursor_x = 0;
                if (cursor_x >= (int)desktop_width)
                    cursor_x = (int)desktop_width - 1;

                if (cursor_y < 0)
                    cursor_y = 0;
                if (cursor_y >= (int)desktop_height)
                    cursor_y = (int)desktop_height - 1;

                button_mask =
                    ((desktop_pressed & PAD_CROSS)  ? 0x01 : 0x00) |
                    ((desktop_pressed & PAD_CIRCLE) ? 0x04 : 0x00);

                if (cursor_x != old_x ||
                    cursor_y != old_y ||
                    button_mask != last_button_mask) {

                    if (send_pointer_event(
                            sock,
                            button_mask,
                            (unsigned int)cursor_x,
                            (unsigned int)cursor_y) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }

                    last_button_mask = button_mask;
                }

                /*
                 * Test11K scroll mode.
                 *
                 * Direction changes fire immediately. Holding the same
                 * direction repeats according to analog deflection.
                 */
                if (scroll_mask != 0) {
                    if (scroll_mask != last_scroll_mask)
                        scroll_repeat_countdown = 0;

                    if (scroll_repeat_countdown == 0) {
                        if (send_scroll_pulse(
                                sock,
                                button_mask,
                                scroll_mask,
                                cursor_x,
                                cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }

                        scroll_repeat_countdown = scroll_delay;
                    } else {
                        scroll_repeat_countdown--;
                    }

                    last_scroll_mask = scroll_mask;
                } else {
                    last_scroll_mask = 0;
                    scroll_repeat_countdown = 0;
                }
            } else {
                int osk_changed = 0;

                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_LEFT | PAD_RIGHT | PAD_UP | PAD_DOWN |
                     PAD_TRIANGLE | PAD_CROSS | PAD_SQUARE |
                     PAD_START | PAD_R1 | PAD_CIRCLE | PAD_SELECT);

                /*
                 * While the OSK is open the D-pad controls the keyboard,
                 * not the remote mouse.  Movement is edge-triggered for
                 * this first version.
                 */
                if ((changed & PAD_LEFT) &&
                    (pressed & PAD_LEFT)) {
                    osk_move_horizontal(-1);
                    osk_changed = 1;
                }

                if ((changed & PAD_RIGHT) &&
                    (pressed & PAD_RIGHT)) {
                    osk_move_horizontal(1);
                    osk_changed = 1;
                }

                if ((changed & PAD_UP) &&
                    (pressed & PAD_UP)) {
                    osk_move_vertical(-1);
                    osk_changed = 1;
                }

                if ((changed & PAD_DOWN) &&
                    (pressed & PAD_DOWN)) {
                    osk_move_vertical(1);
                    osk_changed = 1;
                }

                if ((changed & PAD_TRIANGLE) &&
                    (pressed & PAD_TRIANGLE)) {
                    osk_shift = !osk_shift;
                    osk_generation++;
                    osk_changed = 1;
                }

                if ((changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {
                    if (osk_activate_selected(sock) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    osk_changed = 1;
                }

                if ((changed & PAD_SQUARE) &&
                    (pressed & PAD_SQUARE)) {
                    if (osk_send_key_with_modifiers(
                            sock, XK_BACKSPACE) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    osk_changed = 1;
                }

                if ((changed & PAD_START) &&
                    (pressed & PAD_START)) {
                    if (osk_send_key_with_modifiers(
                            sock, XK_RETURN) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    osk_changed = 1;
                }

                if ((changed & PAD_R1) &&
                    (pressed & PAD_R1)) {
                    if (osk_send_key_with_modifiers(
                            sock, XK_TAB) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    osk_changed = 1;
                }

                if (((changed & PAD_CIRCLE) &&
                     (pressed & PAD_CIRCLE)) ||
                    ((changed & PAD_SELECT) &&
                     (pressed & PAD_SELECT))) {

                    osk_visible = 0;
                    osk_shift = 0;
                    osk_ctrl = 0;
                    osk_alt = 0;
                    osk_generation++;
                    osk_changed = 1;
                }

                /*
                 * Purely local keyboard changes need to wake the main
                 * renderer, which may be blocked in an incremental VNC
                 * request.  Actual typed keys normally cause a remote
                 * redraw themselves.
                 */
                if (osk_changed) {
                    if (wake_renderer_with_pointer_jiggle(
                            sock, cursor_x, cursor_y) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                }
            }

            last_pressed = pressed;
        }

        nanosleep(&poll_delay, NULL);
    }
}

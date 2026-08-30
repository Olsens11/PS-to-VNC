static int ps2vnc_switch_display_mode_core(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    GSGLOBAL **display,
    GSTEXTURE *tex,
    GSTEXTURE *osk_tex,
    const ps2vnc_video_mode_t *mode,
    unsigned int out_x,
    unsigned int out_y,
    unsigned int width,
    unsigned int height)
{
    unsigned int logical_width;

    display_switch_failure_stage = 0;
    live_error_code = 0;
    unsigned int logical_height;

    if (mode == NULL || display == NULL)
{
            display_switch_failure_stage = 1u;
            return -1;
        }

    if (!region_fits_u32(
            out_x, out_y, width, height,
            mode->raster_width, mode->raster_height))
{
            display_switch_failure_stage = 2u;
            return -1;
        }

    /*
     * Main thread remains the sole RFB socket writer. Empty any controller
     * traffic accumulated before entering the display transaction.
     */
    if (rfb_queue_sema >= 0 &&
        flush_rfb_outgoing(sock) < 0)
{
            display_switch_failure_stage = 3u;
            return -1;
        }

    ps2vnc_display_direct_write_cancel();

    if (ps2vnc_display_destroy(display) < 0)
{
            display_switch_failure_stage = 4u;
            return -1;
        }

    active_video_mode = mode;

    *display = ps2vnc_display_create();
    if (*display == NULL)
{
            display_switch_failure_stage = 5u;
            return -1;
        }

    /*
     * Re-advertise EDS because ordinary live mode advertises only
     * Hextile + Raw.
     */
    if (set_raw_extended_desktop_encoding(sock) < 0)
{
            display_switch_failure_stage = 6u;
            return -1;
        }

    if (ps2vnc_display_logical_desktop_size(
            width, height,
            &logical_width, &logical_height) < 0)
{
            display_switch_failure_stage = 7u;
            return -1;
        }

    if (desktop_width != logical_width ||
        desktop_height != logical_height) {
        if (request_and_confirm_desktop_size(
                sock,
                rfb_framebuffer,
                gs_framebuffer,
                logical_width,
                logical_height) < 0)
{
                display_switch_failure_stage = 8u;
                return -1;
            }
    }

    desktop_width = logical_width;
    desktop_height = logical_height;
    desktop_output_x = out_x;
    desktop_output_y = out_y;
    desktop_output_width = width;
    desktop_output_height = height;
    desktop_geometry_locked = 1;

    debug_emit_geometry_snapshot();

    /*
     * Build an authoritative framebuffer at the backend-selected logical
     * desktop geometry before preparing the reconstructed presentation.
     */
    if (set_raw_encoding(sock) < 0)
{
            display_switch_failure_stage = 9u;
            return -1;
        }

    if (request_framebuffer(sock, 0) < 0)
{
            display_switch_failure_stage = 10u;
            return -1;
        }

    if (receive_framebuffer_update(
            sock,
            rfb_framebuffer,
            gs_framebuffer,
            0,
            1) < 0)
{
            display_switch_failure_stage = 11u;
            return -1;
        }

    if (ps2vnc_display_prepare_desktop(
            *display,
            tex,
            gs_framebuffer) < 0)
{
            display_switch_failure_stage = 12u;
            return -1;
        }

    ps2vnc_display_prepare_osk(*display, osk_tex);

    if (set_live_encoding(sock) < 0)
{
            display_switch_failure_stage = 13u;
            return -1;
        }

    /*
     * The OSK texture now belongs to a new GS VRAM instance.
     */
    osk_generation++;

    return 0;
}


/*
 * D17AL-F8J2-B4A — hard remote transition curtain.
 *
 * Every one-way remote test transition follows:
 *
 * controller OFF
 *   -> discard queued controller RFB
 *   -> curtain ON
 *   -> 500 ms source settle
 *   -> existing B3C display reconstruction
 *   -> curtain ON in destination timing
 *   -> 500 ms destination settle
 *   -> complete destination desktop presentation
 *   -> curtain OFF
 *   -> controller ON
 *
 * Forward and rollback both enter through this wrapper. The controller
 * remains fully active during the ordinary temporary-mode dwell.
 *
 * The controller thread never touches the VNC socket here. The main
 * thread remains the sole owner of RFB socket traffic.
 */
int ps2vnc_switch_display_mode(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    GSGLOBAL **display,
    GSTEXTURE *tex,
    GSTEXTURE *osk_tex,
    const ps2vnc_video_mode_t *mode,
    unsigned int out_x,
    unsigned int out_y,
    unsigned int width,
    unsigned int height)
{
    const struct timespec settle_delay = {
        0,
        500000000
    };

    const u64 black =
        GS_SETREG_RGBAQ(
            0x00, 0x00, 0x00,
            0x80, 0x00);

    const u64 tex_color =
        GS_SETREG_RGBAQ(
            0x80, 0x80, 0x80,
            0x80, 0x00);

    int controller_acquired = 0;
    int result = -1;

    /*
     * B4A is deliberately restricted to the backdoor remote display
     * transaction path. Preserve ordinary local UI behavior exactly.
     */
    if (remote_display_state == REMOTE_DISPLAY_IDLE) {
        return ps2vnc_switch_display_mode_core(
            sock,
            rfb_framebuffer,
            gs_framebuffer,
            display,
            tex,
            osk_tex,
            mode,
            out_x,
            out_y,
            width,
            height);
    }

    display_switch_failure_stage = 0;
    live_error_code = 0;

    if (display == NULL ||
        *display == NULL ||
        tex == NULL ||
        osk_tex == NULL ||
        mode == NULL) {
        display_switch_failure_stage = 14u;
        return -1;
    }

    /*
     * True controller disconnect: the controller thread acknowledges
     * immediately before its next libpad access and performs no further
     * padGetState()/padRead() while ownership is held here.
     */
    if (ps2vnc_controller_pad_acquire() < 0) {
        display_switch_failure_stage = 14u;
        return -1;
    }

    controller_acquired = 1;

    /*
     * Drop any pointer/key/scroll messages generated before the pause ACK.
     * Do not send them into the display reconstruction epoch.
     */
    if (ps2vnc_discard_controller_rfb_queue() < 0) {
        display_switch_failure_stage = 15u;
        goto out;
    }

    debug_transition_phase = 1u;
    debug_emit_now();

    /*
     * Source-timing curtain.
     */
    if (draw_recovery_status(
            *display,
            osk_tex,
            "PLEASE WAIT",
            "CHANGING DISPLAY MODES",
            0) < 0) {
        display_switch_failure_stage = 16u;
        goto out;
    }

    debug_transition_phase = 2u;
    debug_emit_now();

    nanosleep(&settle_delay, NULL);

    /*
     * Exact B3C reconstruction body.
     */
    debug_transition_phase = 3u;
    debug_emit_now();

    result = ps2vnc_switch_display_mode_core(
        sock,
        rfb_framebuffer,
        gs_framebuffer,
        display,
        tex,
        osk_tex,
        mode,
        out_x,
        out_y,
        width,
        height);

    if (result < 0)
        goto out;

    debug_transition_phase = 4u;
    debug_emit_now();

    /*
     * The destination GS/RFB reconstruction is now complete. Repaint
     * the same curtain in the destination timing before exposing the
     * reconstructed desktop.
     */
    if (draw_recovery_status(
            *display,
            osk_tex,
            "PLEASE WAIT",
            "CHANGING DISPLAY MODES",
            0) < 0) {
        display_switch_failure_stage = 17u;
        result = -1;
        goto out;
    }

    debug_transition_phase = 5u;
    debug_emit_now();

    nanosleep(&settle_delay, NULL);

    /*
     * Curtain up: explicitly present one coherent destination desktop
     * frame before controller ownership returns.
     */
    if (ps2vnc_display_draw_desktop(
            *display,
            tex,
            black,
            tex_color) < 0) {
        display_switch_failure_stage = 18u;
        result = -1;
        goto out;
    }

    if (ps2vnc_display_finish_draw(*display) < 0) {
        display_switch_failure_stage = 19u;
        result = -1;
        goto out;
    }

    if (ps2vnc_display_flip(*display) < 0) {
        display_switch_failure_stage = 20u;
        result = -1;
        goto out;
    }

    debug_transition_phase = 6u;
    debug_emit_now();

    result = 0;

out:
    /*
     * Do NOT call ps2vnc_controller_pad_wait_release().
     * The hostile-stick benchmark deliberately holds the analog stick.
     * The controller thread's existing resume path clears pre-handoff
     * edge/hold/analog/scroll state before normal polling resumes.
     */
    if (controller_acquired) {
        if (ps2vnc_controller_pad_release() < 0) {
            if (result == 0)
                display_switch_failure_stage = 21u;

            result = -1;
            debug_emit_now();
            return result;
        }

        debug_transition_phase = 7u;
        debug_emit_now();
    }

    debug_transition_phase = 0u;
    debug_emit_now();

    return result;
}


/*
 * H3C2A complete known-good display rollback.
 *
 * The suspect RFB stream is abandoned first. Only after a fresh synchronized
 * session exists do we reconstruct the saved display configuration.
 */
static int ps2vnc_rollback_display_transaction(
    int old_sock,
    int transport_dirty,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    GSGLOBAL **display,
    GSTEXTURE *tex,
    GSTEXTURE *osk_tex)
{
    const ps2vnc_video_mode_t *previous_mode =
        display_confirm_previous_mode;

    const ps2vnc_backend_t previous_backend =
        display_confirm_previous_backend;

    const unsigned int previous_x =
        display_confirm_previous_x;
    const unsigned int previous_y =
        display_confirm_previous_y;
    const unsigned int previous_w =
        display_confirm_previous_w;
    const unsigned int previous_h =
        display_confirm_previous_h;

    const unsigned int previous_desktop_w =
        display_confirm_previous_desktop_w;
    const unsigned int previous_desktop_h =
        display_confirm_previous_desktop_h;

    const int previous_screen_fit_width =
        display_confirm_previous_screen_fit_width;
    const int previous_screen_fit_height =
        display_confirm_previous_screen_fit_height;
    const int previous_screen_fit_offset_x =
        display_confirm_previous_screen_fit_offset_x;
    const int previous_screen_fit_offset_y =
        display_confirm_previous_screen_fit_offset_y;
    const int previous_screen_fit_full_bypass =
        display_confirm_previous_screen_fit_full_bypass;

    int new_sock;

    if (!display_confirm_pending ||
        previous_mode == NULL ||
        previous_backend == PS2VNC_BACKEND_NONE ||
        previous_desktop_w == 0 ||
        previous_desktop_h == 0 ||
        previous_screen_fit_width <= 0 ||
        previous_screen_fit_height <= 0)
        return -1;

    /*
     * Claim rollback before reconnecting. recv_exact() interrupts only the
     * unclaimed ROLLBACK state, never ROLLBACK_IN_PROGRESS.
     */
    display_confirm_action =
        DISPLAY_CONFIRM_ACTION_ROLLBACK_IN_PROGRESS;
    display_confirm_visible = 0;
    osk_generation++;

    /*
     * A completed candidate leaves us at a synchronized RFB message
     * boundary. Go Back / confirmation timeout therefore reuses that
     * healthy session and resizes it directly back to known-good.
     *
     * Only an interrupted candidate or genuine RFB failure owns a suspect
     * byte stream and requires transport replacement first.
     */
    if (transport_dirty) {
        new_sock =
            recover_rfb_transport_for_display_rollback(
                old_sock,
                rfb_framebuffer,
                gs_framebuffer,
                previous_mode);

        if (new_sock < 0)
            return -1;
    } else {
        if (old_sock < 0)
            return -1;

        new_sock = old_sock;
    }

    if (ps2vnc_switch_display_mode(
            new_sock,
            rfb_framebuffer,
            gs_framebuffer,
            display,
            tex,
            osk_tex,
            previous_mode,
            previous_x,
            previous_y,
            previous_w,
            previous_h) < 0) {

        close(new_sock);
        reset_rfb_transport_state();
        return -1;
    }

    /*
     * D17AL-F3:
     * ps2vnc_switch_display_mode() reconstructs A's GS/RFB presentation,
     * but screen-fit state is an independent component of the local profile.
     * Restore it only after A is active again.
     */
    if (!screen_fit_apply_preset(
            previous_screen_fit_width,
            previous_screen_fit_height,
            previous_screen_fit_offset_x,
            previous_screen_fit_offset_y,
            previous_screen_fit_full_bypass)) {

        close(new_sock);
        reset_rfb_transport_state();
        return -1;
    }

    /*
     * Verify that the exact saved known-good configuration came back.
     */
    if (active_video_mode != previous_mode ||
        active_display_backend != previous_backend ||
        desktop_width != previous_desktop_w ||
        desktop_height != previous_desktop_h ||
        desktop_output_x != previous_x ||
        desktop_output_y != previous_y ||
        desktop_output_width != previous_w ||
        desktop_output_height != previous_h ||
        screen_fit_width != previous_screen_fit_width ||
        screen_fit_height != previous_screen_fit_height ||
        screen_fit_offset_x != previous_screen_fit_offset_x ||
        screen_fit_offset_y != previous_screen_fit_offset_y ||
        screen_fit_full_bypass !=
            previous_screen_fit_full_bypass) {

        close(new_sock);
        reset_rfb_transport_state();
        return -1;
    }

    rfb_connected = 1;
    display_confirm_disarm_transaction();

    return new_sock;
}

/*
 * H4B2B:
 * One global rollback boundary for every rollback cause.
 *
 * The durable Pi transaction enters RESTORING before the existing
 * complete local/RFB reconstruction of confirmed profile A. Only after
 * A has been reconstructed successfully may the Pi record RESTORED.
 *
 * If either management transition fails, the best-effort local restore
 * still runs, but the caller receives failure: global reconciliation is
 * incomplete and must never be silently treated as success.
 */
/*
 * Publish the existing H4B2C restored acknowledgement UI.
 *
 * The durable transaction remains RESTORED until controller X causes the
 * main thread to POST ACK successfully.
 */
static void ps2vnc_publish_restored_ack(
    const ps2vnc_video_mode_t *mode)
{
    display_restore_ack_mode =
        (mode != NULL) ? mode : active_video_mode;

    display_restore_ack_request = 0;

    system_menu_visible = 0;
    display_menu_visible = 0;
    osk_visible = 0;

    display_restore_ack_visible = 1;
    osk_generation++;

    screenf(
        "DISPLAY TX: restored %s; awaiting ACK",
        display_restore_ack_mode ?
            display_restore_ack_mode->name : "?");
}


/*
 * Complete durable startup recovery ONLY after the entire local runtime
 * profile exists:
 *
 *   - authoritative mode selected
 *   - authoritative backend created
 *   - TigerVNC at authoritative logical geometry
 *   - output/presentation rectangle installed
 *   - complete Raw framebuffer obtained
 *   - desktop + OSK presentation prepared
 *
 * Capture the live profile and compare it byte-for-field semantically with
 * the selected authority before changing durable transaction state.
 */
int ps2vnc_startup_finalize_display_transaction(void)
{
    ps2vnc_display_profile_t live_profile;
    const ps2vnc_display_profile_t *target;

    if (!startup_display_transaction_loaded)
        return -1;

    if (!startup_authoritative_display_profile_loaded) {

        return startup_display_transaction.state ==
            PS2VNC_DISPLAY_TX_NONE ? 0 : -1;
    }

    target = &startup_authoritative_display_profile;

    if (ps2vnc_display_profile_capture_active(
            &live_profile) < 0 ||
        !ps2vnc_display_tx_profiles_equal(
            &live_profile,
            target)) {

        screenf("DISPLAY TX: runtime profile mismatch");
        return -1;
    }

    switch (startup_display_transaction.state) {

        case PS2VNC_DISPLAY_TX_NONE:

            if (startup_display_commit_finish_required ||
                startup_display_restored_mark_required ||
                startup_display_restore_ack_required) {

                screenf("DISPLAY TX: none obligations invalid");
                return -1;
            }

            return 0;


        case PS2VNC_DISPLAY_TX_COMMITTING:

            if (!startup_display_commit_finish_required) {
                screenf("DISPLAY TX: commit obligation missing");
                return -1;
            }

            /*
             * B is already complete and verified.  The idempotent Pi COMMIT
             * now promotes/persists that already-running B; it does not
             * reconstruct B.
             */
            if (ps2vnc_display_transaction_commit_to_pi() < 0) {
                screenf("DISPLAY TX: startup COMMIT failed");
                return -1;
            }

            startup_display_commit_finish_required = 0;
            startup_display_transaction.state =
                PS2VNC_DISPLAY_TX_NONE;

            screenf(
                "DISPLAY TX: startup committed %s",
                target->mode->name);

            return 0;


        case PS2VNC_DISPLAY_TX_PROVISIONAL:

            /*
             * prepare_display_transaction() must have durably converted this
             * to RESTORING before any reconstruction started.
             */
            screenf("DISPLAY TX: provisional escaped preparation");
            return -1;


        case PS2VNC_DISPLAY_TX_RESTORING:

            if (!startup_display_restored_mark_required ||
                !startup_display_restore_ack_required) {

                screenf("DISPLAY TX: restore obligations invalid");
                return -1;
            }

            /*
             * Complete A is now reconstructed and exactly verified.
             * Only now may the Pi enter durable RESTORED.
             */
            if (ps2vnc_display_transaction_restored_to_pi() < 0) {
                screenf("DISPLAY TX: startup RESTORED failed");
                return -1;
            }

            startup_display_restored_mark_required = 0;
            startup_display_transaction.state =
                PS2VNC_DISPLAY_TX_RESTORED;

            ps2vnc_publish_restored_ack(target->mode);

            return 0;


        case PS2VNC_DISPLAY_TX_RESTORED:

            if (!startup_display_restore_ack_required) {
                screenf("DISPLAY TX: restored ACK obligation missing");
                return -1;
            }

            /*
             * A was already durably RESTORED before this process started.
             * Reassert/verify complete A, but deliberately leave the Pi in
             * RESTORED until the user presses X on the ACK UI.
             */
            ps2vnc_publish_restored_ack(target->mode);

            return 0;


        default:
            return -1;
    }
}



static int ps2vnc_remote_control_get_value(
    const char *text,
    const char *key,
    char *out,
    unsigned int out_size)
{
    const char *line;
    unsigned int key_len;

    if (text == NULL ||
        key == NULL ||
        out == NULL ||
        out_size == 0)
        return 0;

    key_len = (unsigned int)strlen(key);
    line = text;

    while (*line != '\0') {
        const char *end = strchr(line, '\n');
        const char *value;
        unsigned int len;

        if (end == NULL)
            end = line + strlen(line);

        if ((unsigned int)(end - line) > key_len &&
            strncmp(line, key, key_len) == 0 &&
            line[key_len] == '=') {

            value = line + key_len + 1;
            len = (unsigned int)(end - value);

            if (len > 0 &&
                value[len - 1] == '\r')
                len--;

            if (len >= out_size)
                return 0;

            memcpy(out, value, len);
            out[len] = '\0';
            return 1;
        }

        if (*end == '\0')
            break;

        line = end + 1;
    }

    return 0;
}


static int ps2vnc_remote_control_get_u32(
    const char *text,
    const char *key,
    unsigned int *out)
{
    char value[32];
    char *end = NULL;
    unsigned long parsed;

    if (out == NULL ||
        !ps2vnc_remote_control_get_value(
            text,
            key,
            value,
            sizeof(value)))
        return 0;

    if (value[0] == '\0')
        return 0;

    parsed = strtoul(value, &end, 10);

    if (end == value ||
        *end != '\0')
        return 0;

    *out = (unsigned int)parsed;
    return 1;
}


int ps2vnc_remote_result_post(
    const char *state,
    unsigned int error_code)
{
    char body[512];
    int n;

    const char *active_name =
        (active_video_mode != NULL &&
         active_video_mode->name != NULL)
            ? active_video_mode->name
            : "";

    const char *baseline_name =
        (remote_display_baseline_mode != NULL &&
         remote_display_baseline_mode->name != NULL)
            ? remote_display_baseline_mode->name
            : active_name;

    if (remote_display_txid == 0 ||
        state == NULL)
        return -1;

    n = snprintf(
        body,
        sizeof(body),
        "txid=%u\n"
        "state=%s\n"
        "active_mode=%s\n"
        "baseline_mode=%s\n"
        "error_code=%u\n",
        remote_display_txid,
        state,
        active_name,
        baseline_name,
        error_code);

    if (n <= 0 ||
        n >= (int)sizeof(body))
        return -1;

    return ps2vnc_management_post_text(
        PS2VNC_DISPLAY_CONTROL_RESULT_PATH,
        body);
}


static void ps2vnc_remote_display_reset(void)
{
    remote_display_state = REMOTE_DISPLAY_IDLE;
    display_mode_request_remote = 0;

    remote_display_txid = 0;
    remote_display_lease_seconds = 0;
    remote_display_error_code = 0;

    remote_display_lease_started_tick = 0;

    remote_display_requested_mode = NULL;
    remote_display_baseline_mode = NULL;
}


void ps2vnc_remote_display_fail(
    unsigned int error_code)
{
    remote_display_error_code = error_code;

    (void)ps2vnc_remote_result_post(
        "FAILED",
        error_code);

    ps2vnc_remote_display_reset();
}


 int ps2vnc_remote_display_lease_expired(void)
{
    u32 sec = 0;
    u32 usec = 0;

    if (remote_display_state !=
            REMOTE_DISPLAY_ACTIVE_TEMP ||
        !display_confirm_pending ||
        display_confirm_action !=
            DISPLAY_CONFIRM_ACTION_NONE)
        return 0;

    TimerBusClock2USec(
        GetTimerSystemTime() -
            remote_display_lease_started_tick,
        &sec,
        &usec);

    if (sec < remote_display_lease_seconds)
        return 0;

    screenf(
        "REMOTE DISPLAY: lease expired tx=%u",
        remote_display_txid);

    remote_display_state =
        REMOTE_DISPLAY_RESTORING;

    remote_display_error_code = 0;

    (void)ps2vnc_remote_result_post(
        "RESTORING",
        0);

    display_confirm_action =
        DISPLAY_CONFIRM_ACTION_ROLLBACK;

    return 1;
}


 int ps2vnc_remote_control_poll_idle(void)
{
    unsigned char body[
        PS2VNC_DISPLAY_CONTROL_MAX_BYTES + 1];

    unsigned int len = 0;
    unsigned int txid;
    unsigned int lease_seconds;

    char state[32];
    char action[32];
    char mode_name[64];

    const ps2vnc_video_mode_t *target;
    u64 now = GetTimerSystemTime();

    if (remote_display_last_poll_tick != 0 &&
        profile_ticks_us(
            now -
            remote_display_last_poll_tick) <
            PS2VNC_REMOTE_CONTROL_POLL_US)
        return 0;

    remote_display_last_poll_tick = now;

    if (ps2vnc_management_get_bounded_quiet(
            PS2VNC_DISPLAY_CONTROL_PENDING_PATH,
            body,
            PS2VNC_DISPLAY_CONTROL_MAX_BYTES,
            &len,
            PS2VNC_REMOTE_CONTROL_GET_BUDGET_US) < 0) {

        /*
         * Development control is best-effort.
         * A missing Pi management service must never damage VNC.
         */
        return 0;
    }

    if (len == 0 ||
        len > PS2VNC_DISPLAY_CONTROL_MAX_BYTES)
        return 0;

    body[len] = '\0';

    if (!ps2vnc_remote_control_get_value(
            (const char *)body,
            "state",
            state,
            sizeof(state)))
        return 0;

    if (strcmp(state, "none") == 0)
        return 0;

    if (!ps2vnc_remote_control_get_u32(
            (const char *)body,
            "txid",
            &txid) ||
        !ps2vnc_remote_control_get_value(
            (const char *)body,
            "action",
            action,
            sizeof(action)))
        return 0;

    if (strcmp(state, "PENDING") == 0 &&
        strcmp(action, "set-temp") == 0) {

        /*
         * Wait rather than steal ownership from a local transaction.
         */
        if (display_confirm_pending ||
            display_transition_switch_in_progress ||
            display_mode_request !=
                DISPLAY_MODE_REQUEST_NONE ||
            display_calibration_request !=
                DISPLAY_MODE_REQUEST_NONE ||
            controller_pad_pause_requested ||
            remote_display_state !=
                REMOTE_DISPLAY_IDLE)
            return 0;

        if (!ps2vnc_remote_control_get_value(
                (const char *)body,
                "mode",
                mode_name,
                sizeof(mode_name)) ||
            !ps2vnc_remote_control_get_u32(
                (const char *)body,
                "lease_seconds",
                &lease_seconds) ||
            lease_seconds == 0)
            return 0;

        target =
            pstvnc_video_mode_by_name(mode_name);

        remote_display_txid = txid;
        remote_display_lease_seconds =
            lease_seconds;
        remote_display_error_code = 0;

        remote_display_requested_mode = target;
        remote_display_baseline_mode =
            active_video_mode;

        if (target == NULL) {
            ps2vnc_remote_display_fail(1);
            return 0;
        }

        if (target == active_video_mode) {
            ps2vnc_remote_display_fail(2);
            return 0;
        }

        remote_display_state =
            REMOTE_DISPLAY_SWITCH_REQUESTED;

        display_mode_request_remote = 1;
        display_mode_request =
            display_mode_request_for_selection(
                display_menu_selection_for_mode(
                    target));

        if (display_mode_request ==
                DISPLAY_MODE_REQUEST_NONE) {
            ps2vnc_remote_display_fail(3);
            return 0;
        }

        screenf(
            "REMOTE DISPLAY: tx=%u set-temp %s lease=%us",
            txid,
            target->name,
            lease_seconds);

        return 1;
    }

    if (strcmp(state, "RESTORE_PENDING") == 0 &&
        strcmp(action, "restore") == 0) {

        if (remote_display_state !=
                REMOTE_DISPLAY_ACTIVE_TEMP ||
            txid != remote_display_txid)
            return 0;

        remote_display_state =
            REMOTE_DISPLAY_RESTORING;

        remote_display_error_code = 0;

        (void)ps2vnc_remote_result_post(
            "RESTORING",
            0);

        display_confirm_action =
            DISPLAY_CONFIRM_ACTION_ROLLBACK;

        screenf(
            "REMOTE DISPLAY: explicit restore tx=%u",
            txid);

        return 1;
    }

    return 0;
}


int ps2vnc_global_rollback_display_transaction(
    int old_sock,
    int transport_dirty,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    GSGLOBAL **display,
    GSTEXTURE *tex,
    GSTEXTURE *osk_tex)
{
    int restore_marked;
    int new_sock;

    const int remote_restore =
        remote_display_state != REMOTE_DISPLAY_IDLE;

    if (remote_restore) {
        remote_display_suppress_next_active_menu_return = 1;

        remote_display_state =
            REMOTE_DISPLAY_RESTORING;

        (void)ps2vnc_remote_result_post(
            "RESTORING",
            remote_display_error_code);
    }

    restore_marked =
        ps2vnc_display_transaction_restore_to_pi() == 0;

    if (!restore_marked)
        screenf("DISPLAY TX: restore marker failed");

    new_sock = ps2vnc_rollback_display_transaction(
        old_sock,
        transport_dirty,
        rfb_framebuffer,
        gs_framebuffer,
        display,
        tex,
        osk_tex);

    if (new_sock < 0) {
        if (remote_restore)
            (void)ps2vnc_remote_result_post(
                "FAILED",
                remote_display_error_code != 0
                    ? remote_display_error_code
                    : 90);
        return -1;
    }

    if (!restore_marked) {
        screenf("DISPLAY TX: Pi reconciliation pending");

        if (remote_restore)
            (void)ps2vnc_remote_result_post(
                "FAILED",
                remote_display_error_code != 0
                    ? remote_display_error_code
                    : 91);

        return -1;
    }

    if (ps2vnc_display_transaction_restored_to_pi() < 0) {
        screenf("DISPLAY TX: restored marker failed");

        if (remote_restore)
            (void)ps2vnc_remote_result_post(
                "FAILED",
                remote_display_error_code != 0
                    ? remote_display_error_code
                    : 92);

        return -1;
    }

    if (remote_restore) {
        /*
         * Remote test rollback is machine-owned. The runtime is already
         * back at complete A and Pi is durably RESTORED, so finish ACK
         * without presenting the human restored-acknowledgement modal.
         */
        if (ps2vnc_display_transaction_ack_to_pi() < 0) {
            screenf(
                "REMOTE DISPLAY: automatic ACK failed");

            (void)ps2vnc_remote_result_post(
                "FAILED",
                remote_display_error_code != 0
                    ? remote_display_error_code
                    : 93);

            /*
             * Do not hide unresolved durability from the user.
             */
            ps2vnc_publish_restored_ack(
                active_video_mode);

            ps2vnc_remote_display_reset();
            return new_sock;
        }

        if (remote_display_error_code != 0) {
            (void)ps2vnc_remote_result_post(
                "FAILED",
                remote_display_error_code);
        } else {
            (void)ps2vnc_remote_result_post(
                "RESTORED",
                0);
        }

        screenf(
            "REMOTE DISPLAY: restored %s",
            active_video_mode ?
                active_video_mode->name : "?");

        ps2vnc_remote_display_reset();

        return new_sock;
    }

    ps2vnc_publish_restored_ack(
        (display_confirm_previous_mode != NULL)
            ? display_confirm_previous_mode
            : active_video_mode);

    return new_sock;
}

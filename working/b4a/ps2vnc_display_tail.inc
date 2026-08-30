static int display_live_desktop(int sock,
                                unsigned short *rfb_framebuffer,
                                unsigned short *gs_framebuffer)
{
    GSGLOBAL *gsGlobal;
    GSTEXTURE tex;
    GSTEXTURE osk_tex;
    unsigned int rendered_osk_generation = 0;

    /*
     * D17AL-F4:
     * A local confirmation redraw does not consume or cancel the current
     * incremental FramebufferUpdateRequest.
     */
    int rfb_update_request_outstanding = 0;

    const u64 black =
        GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x00);

    const u64 tex_color =
        GS_SETREG_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x00);

    /*
     * DMAKit remains alive across future display-mode reconstruction.
     * Preserve the proven one-time initialization ordering for E1-B.
     */
    dmaKit_init(
        D_CTRL_RELE_OFF,
        D_CTRL_MFD_OFF,
        D_CTRL_STS_UNSPEC,
        D_CTRL_STD_OFF,
        D_CTRL_RCYC_8,
        1 << DMA_CHANNEL_GIF
    );

    dmaKit_chan_init(DMA_CHANNEL_GIF);

    gsGlobal = ps2vnc_display_create();
    if (gsGlobal == NULL) {
        screenf("ERROR: display initialization failed");
        return -1;
    }

    /*
     * TEST13J-A:
     * Determine the centered area the television actually exposes before
     * publishing the VNC desktop. No VNC geometry changes are made yet.
     */
    if (!ps2vnc_display_config_loaded ||
        force_screen_fit_calibration) {

        if (run_screen_fit_calibration(
                gsGlobal,
                black,
                1,
                NULL) < 0)
            return -1;

        /*
         * Startup override is one-shot.  A later reconnect in this same
         * session must not unexpectedly reopen calibration.
         */
        force_screen_fit_calibration = 0;
    }

    /*
     * TEST13J-D3-A2:
     *
     * Keep the physical GS output at 1280x720, but make TigerVNC itself
     * render only the guaranteed-visible calibrated desktop.
     */
    {
        unsigned int target_x;
        unsigned int target_y;
        unsigned int target_w;
        unsigned int target_h;
        unsigned int logical_w;
        unsigned int logical_h;

        if (calculate_presented_desktop_geometry(
                &target_x,
                &target_y,
                &target_w,
                &target_h) < 0) {

            screenf("ERROR: invalid calibrated desktop geometry");
            return -1;
        }

        /*
         * D2 already proved this SetDesktopSize + 1x1 acknowledgement
         * path. Resize only when the server is not already at the target.
         */
        if (ps2vnc_display_logical_desktop_size(
                target_w, target_h,
                &logical_w, &logical_h) < 0)
            return -1;

        if (desktop_width != logical_w ||
            desktop_height != logical_h) {

            if (request_and_confirm_desktop_size(
                    sock,
                    rfb_framebuffer,
                    gs_framebuffer,
                    logical_w,
                    logical_h) < 0) {

                return -1;
            }
        }

        /*
         * From this point onward the RFB framebuffer is tightly packed at
         * the backend-selected logical desktop width. HIRES uses the
         * calibrated width; Standard uses its fixed framebuffer width.
         */
        desktop_width = logical_w;
        desktop_height = logical_h;

        desktop_output_x = target_x;
        desktop_output_y = target_y;
        desktop_output_width = target_w;
        desktop_output_height = target_h;

        debug_emit_geometry_snapshot();

        desktop_geometry_locked = 1;

        /*
         * Stop requesting EDS during normal rendering and obtain a clean
         * complete Raw framebuffer at the NEW desktop geometry.
         *
         * Unlike D2/A1, there is deliberately NO restore to 1280x720.
         */
        if (set_raw_encoding(sock) < 0)
            return -1;

        if (request_framebuffer(sock, 0) < 0)
            return -1;

        if (receive_framebuffer_update(
                sock,
                rfb_framebuffer,
                gs_framebuffer,
                0,
                1) < 0) {

            return -1;
        }
    }

    if (ps2vnc_display_prepare_desktop(
            gsGlobal, &tex, gs_framebuffer) < 0) {
        screenf("ERROR: HIRES presentation setup failed");
        return -1;
    }

    if (set_live_encoding(sock) < 0) {
        screenf("ERROR: live SetEncodings failed");
        return -1;
    }

    ps2vnc_display_prepare_osk(gsGlobal, &osk_tex);

    /*
     * H4C3C3C:
     * The complete runtime display now exists.  Only at this boundary may
     * startup recovery promote B or mark restored A durable.
     */
    if (ps2vnc_startup_finalize_display_transaction() < 0) {
        screenf("DISPLAY TX: startup finalization failed");
        return -1;
    }

    /*
     * The video path below remains the known-good Milestone 6 blocking
     * incremental loop.  Controller input runs independently.
     */
    pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_CONTROLLER_START);

    rfb_connected = 1;

    if (start_controller_thread(sock) < 0) {
        rfb_connected = 0;
        return -1;
    }

    for (;;) {
        if (exit_requested)
            return 1;

        debug_frame_counter++;
        pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_LOOP_BEGIN);
        debug_changed_bytes = 0;

        /*
         * Remote control is serviced at complete main-loop boundaries and
         * also from the idle EAGAIN path. Lease expiration is PS2-owned.
         */
        (void)ps2vnc_remote_display_lease_expired();
        (void)ps2vnc_remote_control_poll_idle();

        /*
         * TEST15E3-E:
         * Manual Refresh can arrive between RFB transactions.  If it arrives
         * inside recv_exact(), that function interrupts the suspect stream and
         * the receive-error branch below performs this same transaction.
         */
        if (runtime_action_request ==
            RUNTIME_ACTION_REFRESH_RFB) {

            int recovered_sock;

            debug_emit_now();

            recovered_sock =
                perform_manual_rfb_recovery(
                    sock,
                    rfb_framebuffer,
                    gs_framebuffer,
                    gsGlobal,
                    &tex,
                    &osk_tex
                );

            if (recovered_sock < 0) {
                if (exit_requested)
                    return 1;

                return -1;
            }

            sock = recovered_sock;

            rendered_osk_generation =
                osk_generation - 1;

            continue;
        }

        /*
         * H4B2C:
         * Controller X publishes only an acknowledgement request.  The main
         * RFB/render thread owns the management socket and releases this UI
         * only after the Pi confirms ACK with HTTP 200.
         */
        if (display_restore_ack_visible &&
            display_restore_ack_request) {

            display_restore_ack_request = 0;

            if (ps2vnc_display_transaction_ack_to_pi() < 0) {
                screenf("DISPLAY TX: ACK failed; retry with X");

                osk_generation++;
                rendered_osk_generation =
                    osk_generation - 1;
                continue;
            }

            screenf(
                "DISPLAY TX: ACK complete %s",
                display_restore_ack_mode ?
                    display_restore_ack_mode->name : "?");

            /*
             * Withdraw visible ownership before reopening Display Modes.
             */
            display_restore_ack_visible = 0;
            display_restore_ack_mode = NULL;

            display_confirm_return_to_active_menu();

            rendered_osk_generation =
                osk_generation - 1;
            continue;
        }

        /*
         * TEST15E3K A14-G2-D2B:
         * Same-mode calibration is a main-thread transaction.  Consume it at
         * the same proven between-RFB boundary used by runtime mode switching,
         * before the next incremental framebuffer request can be issued.
         */

        /*
         * D17AL-F2:
         *
         * Display admission/UI policy uses the same main-thread management
         * ownership boundary as calibration and mode transactions.
         */
        if (display_policy_request !=
                DISPLAY_POLICY_REQUEST_NONE) {

            int request =
                display_policy_request;

            int selection =
                display_policy_request_selection;

            int value =
                display_policy_request_value;

            int policy_result = -1;

            if (request ==
                    DISPLAY_POLICY_REQUEST_REFRESH) {

                policy_result =
                    ps2vnc_display_policy_refresh_from_pi();

            } else if (
                request ==
                    DISPLAY_POLICY_REQUEST_LOCK) {

                if (display_policy_loaded &&
                    selection >= 0 &&
                    (unsigned int)selection <
                        VIDEO_MODE_COUNT &&
                    video_modes[selection] !=
                        active_video_mode) {

                    policy_result =
                        ps2vnc_display_policy_lock_to_pi(
                            selection,
                            value);

                    if (policy_result == 0)
                        display_mode_locked[selection] =
                            value ? 1 : 0;
                }

            } else if (
                request ==
                    DISPLAY_POLICY_REQUEST_HIDE_LOCKED) {

                if (display_policy_loaded) {
                    policy_result =
                        ps2vnc_display_policy_hide_to_pi(
                            value);

                    if (policy_result == 0)
                        display_hide_locked_modes =
                            value ? 1 : 0;
                }
            }

            if (policy_result < 0) {
                display_policy_error = 1;

            } else {
                display_policy_error = 0;
            }

            display_policy_request_selection = -1;
            display_policy_request_value = 0;
            display_policy_request =
                DISPLAY_POLICY_REQUEST_NONE;

            display_menu_normalize_selection();

            osk_generation++;
            rendered_osk_generation =
                osk_generation - 1;

            continue;
        }


        if (display_calibration_request != DISPLAY_MODE_REQUEST_NONE) {
            int request = display_calibration_request;
            const ps2vnc_video_mode_t *target_mode;

            display_calibration_request = DISPLAY_MODE_REQUEST_NONE;
            target_mode = display_mode_request_target(request);

            /*
             * The request is valid only while it still names the active mode.
             * Never reinterpret a stale calibration request as a mode switch.
             */
            if (target_mode == NULL ||
                target_mode != active_video_mode) {

                display_menu_selection =
                    display_menu_selection_for_mode(active_video_mode);
                display_menu_error = 1;
                display_menu_visible = 1;
                display_menu_parent_system = 0;
                osk_generation++;
                rendered_osk_generation = osk_generation - 1;
                continue;
            }

            if (ps2vnc_run_runtime_calibration(
                    sock,
                    rfb_framebuffer,
                    gs_framebuffer,
                    gsGlobal,
                    &tex,
                    black) < 0) {

                return -1;
            }

            /*
             * Return directly to the active row so X once again means
             * "Calibrate <mode> Safe Area".
             */
            display_menu_selection =
                display_menu_selection_for_mode(active_video_mode);
            display_menu_error = 0;
            display_menu_visible = 1;
            display_menu_parent_system = 0;
            osk_generation++;
            rendered_osk_generation = osk_generation - 1;
            continue;
        }

        /*
         * H3C2B1: a provisional display owns this main-loop transaction until
         * the user explicitly keeps it or rollback reconstructs known-good.
         */
        if (display_confirm_pending &&
            display_confirm_action == DISPLAY_CONFIRM_ACTION_KEEP) {

            /*
             * H4B2A:
             * B is already the complete active runtime profile. KEEP does
             * not reconstruct anything. It only asks the Pi to promote the
             * already-running provisional transaction to confirmed state.
             */
            if (ps2vnc_display_transaction_commit_to_pi() < 0) {

                /*
                 * Do not silently accept a mode whose global persistence
                 * commit was not acknowledged. Return ownership to the
                 * confirmation UI so the user may explicitly retry.
                 */
                screenf("DISPLAY TX: commit failed");

                display_confirm_action =
                    DISPLAY_CONFIRM_ACTION_NONE;
                display_confirm_selection =
                    DISPLAY_CONFIRM_KEEP;
                display_confirm_visible = 1;

                osk_generation++;
                rendered_osk_generation =
                    osk_generation - 1;
                continue;
            }

            screenf(
                "DISPLAY TX: committed %s",
                active_video_mode->name);

            display_confirm_disarm_transaction();
            display_confirm_return_to_active_menu();
            rendered_osk_generation = osk_generation - 1;
            continue;
        }

        if (display_confirm_pending &&
            display_confirm_action == DISPLAY_CONFIRM_ACTION_ROLLBACK) {

            int rollback_sock =
                ps2vnc_global_rollback_display_transaction(
                    sock,
                    0,
                    rfb_framebuffer,
                    gs_framebuffer,
                    &gsGlobal,
                    &tex,
                    &osk_tex);

            if (rollback_sock < 0)
                return -1;

            sock = rollback_sock;

            /*
             * F8J2-B3B:
             *
             * A successful rollback reconstruction is an RFB request-epoch
             * boundary exactly like a successful forward mode switch.
             *
             * Explicit remote restore can arrive through the benign
             * RFB_RECEIVE_LOCAL_REDRAW path.  That path correctly preserves
             * the PRE-ROLLBACK incremental request while yielding to the
             * transaction owner, but the request cannot remain authoritative
             * after ps2vnc_global_rollback_display_transaction() has consumed
             * its own EDS/full-frame reconstruction of the previous mode.
             *
             * Force the first restored live iteration to issue a fresh
             * incremental FramebufferUpdateRequest for the restored geometry.
             */
            rfb_update_request_outstanding = 0;

            display_confirm_return_to_active_menu();
            rendered_osk_generation = osk_generation - 1;
            continue;
        }

        /*
         * TEST15E2-D2B:
         * Consume local controller requests on the main RFB/render thread.
         *
         * Target configuration and geometry are resolved before entering
         * ps2vnc_switch_display_mode(), so an unavailable/invalid preset
         * leaves the current GS display completely untouched.
         */
        if (display_mode_request != DISPLAY_MODE_REQUEST_NONE) {
            int request = display_mode_request;
            int request_is_remote =
                display_mode_request_remote;
            int display_switch_result;
            const ps2vnc_video_mode_t *target_mode = NULL;
            ps2vnc_display_config_t target_config;
            unsigned int target_x;
            unsigned int target_y;
            unsigned int target_w;
            unsigned int target_h;

            ps2vnc_display_profile_t previous_profile;
            ps2vnc_display_profile_t candidate_profile;

            /*
             * Claim the request before doing any blocking work.  The
             * controller may publish a later request only after the user
             * opens the selector again.
             */
            display_mode_request = DISPLAY_MODE_REQUEST_NONE;
            display_mode_request_remote = 0;

            target_mode =
                display_mode_request_target(request);

            /*
             * Same-mode requests are defined as harmless no-ops.
             * The controller normally filters these already.
             */
            if (target_mode == active_video_mode) {
                if (request_is_remote)
                    ps2vnc_remote_display_fail(4);

                continue;
            }


            /*
             * Lock policy is checked again at the sole GS-switch owner.
             * A stale or malformed controller request therefore still cannot
             * construct a locked candidate.
             */
            if (target_mode == NULL ||
                (!request_is_remote &&
                 (!display_policy_loaded ||
                  display_mode_locked[
                    display_menu_selection_for_mode(
                        target_mode)]))) {

                display_menu_selection =
                    display_menu_selection_for_mode(
                        target_mode != NULL
                            ? target_mode
                            : active_video_mode);

                display_menu_tab =
                    display_menu_tab_for_mode(
                        target_mode != NULL
                            ? target_mode
                            : active_video_mode);

                display_menu_error = 1;
                display_menu_visible = 1;
                display_menu_parent_system = 0;

                osk_generation++;
                rendered_osk_generation =
                    osk_generation - 1;

                continue;
            }

            if (target_mode == NULL ||
                !ps2vnc_config_get_display_for_mode(
                    target_mode, &target_config) ||
                ps2vnc_config_presented_geometry_for_mode(
                    target_mode,
                    &target_config,
                    &target_x,
                    &target_y,
                    &target_w,
                    &target_h) < 0) {

                if (request_is_remote) {
                    ps2vnc_remote_display_fail(5);
                    continue;
                }

                display_menu_selection =
                    display_menu_selection_for_mode(target_mode);
                display_menu_error = 1;
                display_menu_visible = 1;
                osk_generation++;
                rendered_osk_generation = osk_generation - 1;
                continue;
            }

            /*
             * Build both complete cross-machine profiles while A is still
             * unquestionably active. No runtime or persistence state has
             * changed yet.
             */
            if (ps2vnc_display_profile_capture_active(
                    &previous_profile) < 0 ||
                ps2vnc_display_profile_resolve_candidate(
                    target_mode,
                    target_x,
                    target_y,
                    target_w,
                    target_h,
                    &candidate_profile) < 0) {

                if (request_is_remote) {
                    ps2vnc_remote_display_fail(6);
                    continue;
                }

                screenf("DISPLAY TX: profile resolution failed");

                display_menu_selection =
                    display_menu_selection_for_mode(
                        active_video_mode);
                display_menu_error = 1;
                display_menu_visible = 1;

                osk_generation++;
                rendered_osk_generation =
                    osk_generation - 1;
                continue;
            }

            /*
             * Global transaction record FIRST.
             *
             * If BEGIN is refused or the management service is unavailable,
             * A remains fully active on both machines and no risky display
             * reconstruction begins.
             */
            if (request_is_remote) {
                remote_display_state =
                    REMOTE_DISPLAY_SWITCHING;

                remote_display_error_code = 0;

                (void)ps2vnc_remote_result_post(
                    "SWITCHING",
                    0);
            }

            if (ps2vnc_display_transaction_begin_to_pi(
                    &previous_profile,
                    &candidate_profile) < 0) {

                screenf("DISPLAY TX: begin failed");

                if (request_is_remote) {
                    ps2vnc_remote_display_fail(7);
                    continue;
                }

                display_menu_selection =
                    display_menu_selection_for_mode(
                        active_video_mode);
                display_menu_error = 1;
                display_menu_visible = 1;

                osk_generation++;
                rendered_osk_generation =
                    osk_generation - 1;
                continue;
            }

            /*
             * The Pi now durably knows A is confirmed and B is provisional.
             * Arm the local timeout/rollback obligation before touching GS
             * or RFB runtime state.
             */
            display_confirm_arm_transaction(
                active_video_mode,
                desktop_output_x,
                desktop_output_y,
                desktop_output_width,
                desktop_output_height,
                screen_fit_width,
                screen_fit_height,
                screen_fit_offset_x,
                screen_fit_offset_y,
                screen_fit_full_bypass);

            display_transition_switch_in_progress = 1;

            display_switch_result =
                ps2vnc_switch_display_mode(
                    sock,
                    rfb_framebuffer,
                    gs_framebuffer,
                    &gsGlobal,
                    &tex,
                    &osk_tex,
                    target_mode,
                    target_x,
                    target_y,
                    target_w,
                    target_h);

            /*
             * F8J2-B3A:
             *
             * The synchronous display reconstruction owns its own EDS/full
             * framebuffer requests and consumes the authoritative candidate
             * frame before returning.
             *
             * A remote display request may have arrived by yielding from an
             * older incremental FramebufferUpdateRequest at a clean message
             * boundary.  That caller-local request bookkeeping belongs to the
             * PRE-SWITCH geometry and must not survive reconstruction.
             *
             * Re-arm live incremental traffic unconditionally after the
             * synchronous switch attempt.  Success must send a fresh request
             * for the new geometry; failure/rollback must likewise never
             * inherit the abandoned pre-switch request.
             */
            rfb_update_request_outstanding = 0;


            /*
             * Clear this before inspecting the result.  Once the synchronous
             * call has returned, all failures and elapsed deadlines belong to
             * the existing graceful main-thread rollback machinery.
             */
            display_transition_switch_in_progress = 0;

            if (display_switch_result < 0) {

                int rollback_sock;

                if (request_is_remote) {
                    unsigned int diagnostic_live =
                        live_error_code > 999
                            ? 999u
                            : (unsigned int)live_error_code;

                    remote_display_error_code =
                        800000u +
                        display_switch_failure_stage * 1000u +
                        diagnostic_live;
                }

                /*
                 * Candidate failed or was interrupted by blind-X/timeout.
                 * Never leave the partially applied configuration active.
                 */
                display_confirm_action =
                    DISPLAY_CONFIRM_ACTION_ROLLBACK;

                rollback_sock =
                    ps2vnc_global_rollback_display_transaction(
                        sock,
                        1,
                    rfb_framebuffer,
                        gs_framebuffer,
                        &gsGlobal,
                        &tex,
                        &osk_tex);

                if (rollback_sock < 0)
                    return -1;

                sock = rollback_sock;

                display_confirm_return_to_active_menu();
                rendered_osk_generation = osk_generation - 1;
                continue;
            }

            /*
             * D17AL-F3:
             * The GS/RFB candidate is now complete. Publish the already-
             * validated mode-specific calibration into local screen-fit
             * authority before exposing B to confirmation.
             *
             * target_config was fetched and validated while A was still
             * active. Any unexpected publication mismatch is treated exactly
             * like a candidate failure and rolls back the complete profile.
             */
            {
                unsigned int verify_x;
                unsigned int verify_y;
                unsigned int verify_w;
                unsigned int verify_h;

                if (!ps2vnc_config_apply_display(&target_config) ||
                    calculate_presented_desktop_geometry(
                        &verify_x,
                        &verify_y,
                        &verify_w,
                        &verify_h) < 0 ||
                    verify_x != target_x ||
                    verify_y != target_y ||
                    verify_w != target_w ||
                    verify_h != target_h) {

                    int rollback_sock;

                    if (request_is_remote)
                        remote_display_error_code = 9;

                    screenf(
                        "DISPLAY TX: candidate calibration apply failed");

                    display_confirm_action =
                        DISPLAY_CONFIRM_ACTION_ROLLBACK;

                    rollback_sock =
                        ps2vnc_global_rollback_display_transaction(
                            sock,
                            0,
                            rfb_framebuffer,
                            gs_framebuffer,
                            &gsGlobal,
                            &tex,
                            &osk_tex);

                    if (rollback_sock < 0)
                        return -1;

                    sock = rollback_sock;

                    display_confirm_return_to_active_menu();
                    rendered_osk_generation =
                        osk_generation - 1;
                    continue;
                }
            }

            if (request_is_remote) {
                remote_display_state =
                    REMOTE_DISPLAY_ACTIVE_TEMP;

                remote_display_lease_started_tick =
                    GetTimerSystemTime();

                /*
                 * Keep the durable transaction provisional. Do not expose
                 * KEEP/COMMIT confirmation UI and do not mutate startup_mode.
                 */
                display_confirm_visible = 0;
                display_confirm_seconds = 0;

                (void)ps2vnc_remote_result_post(
                    "ACTIVE_TEMP",
                    0);

                screenf(
                    "REMOTE DISPLAY: ACTIVE_TEMP %s tx=%u lease=%us",
                    active_video_mode->name,
                    remote_display_txid,
                    remote_display_lease_seconds);

                continue;
            }

            /*
             * The candidate is fully reconstructed but remains provisional.
             * The original 30-second deadline has continued running during
             * the switch itself.
             */
            display_confirm_seconds =
                display_confirm_remaining_seconds();

            if (display_confirm_seconds == 0 ||
                display_confirm_action ==
                    DISPLAY_CONFIRM_ACTION_ROLLBACK) {

                int rollback_sock;

                display_confirm_action =
                    DISPLAY_CONFIRM_ACTION_ROLLBACK;

                rollback_sock =
                    ps2vnc_global_rollback_display_transaction(
                        sock,
                        0,
                    rfb_framebuffer,
                        gs_framebuffer,
                        &gsGlobal,
                        &tex,
                        &osk_tex);

                if (rollback_sock < 0)
                    return -1;

                sock = rollback_sock;

                display_confirm_return_to_active_menu();
                rendered_osk_generation = osk_generation - 1;
                continue;
            }

            display_confirm_selection =
                DISPLAY_CONFIRM_GO_BACK;
            display_confirm_visible = 1;

            system_menu_visible = 0;
            display_menu_visible = 0;
            osk_visible = 0;

            osk_generation++;
            rendered_osk_generation = osk_generation - 1;
            continue;
        }

        if ((debug_frame_counter % 60) == 0) {
            debug_emit_now();
            profile_emit_now();
        }

        pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_PRE_DRAW);

        /*
         * HIRES already has its desktop background configured. Standard
         * instead clears and draws its conventional desktop texture here.
         */
        if (ps2vnc_display_draw_desktop(
                gsGlobal,
                &tex,
                black,
                tex_color) < 0) {
            live_error_code = 208;
            return -1;
        }

        /*
         * Render/upload the OSK texture only when its local state changed.
         * Drawing it is just one additional textured sprite.
         */
        if (osk_visible ||
            display_menu_visible ||
            system_menu_visible ||
            display_confirm_visible ||
            display_restore_ack_visible) {

            if (rendered_osk_generation != osk_generation) {
                /*
                 * Display confirmation is safety-critical and owns the
                 * overlay whenever visible. It must not be hidden beneath
                 * the ordinary system/display menus or OSK.
                 */
                if (display_restore_ack_visible)
                    display_restore_ack_render_bitmap();
                else if (display_confirm_visible)
                    display_confirm_render_bitmap();
                else if (system_menu_visible)
                    system_menu_render_bitmap();
                else if (display_menu_visible)
                    display_menu_render_bitmap();
                else
                    osk_render_bitmap();

                /*
                 * Prevent an immediate OSK texture upload from racing
                 * the previous HIRES frame.
                 */
                if (ps2vnc_display_wait_texture_idle(gsGlobal) < 0)
                    return -1;

                gsKit_texture_upload(gsGlobal, &osk_tex);

                rendered_osk_generation = osk_generation;
            }

            {
                float dst_x0;
                float dst_y0;
                float dst_x1;
                float dst_y1;

                {
                    float safe_x0 = 0.0f;
                    float safe_y0 = 0.0f;
                    float safe_w = (float)OUTPUT_WIDTH;
                    float safe_h = (float)OUTPUT_HEIGHT;

                    float scale_x;
                    float scale_y;
                    float scale;
                    float dst_w;
                    float dst_h;

                    /*
                     * D17AL-F1:
                     *
                     * Ordinary local PS2VNC UI is contained by the calibrated
                     * safe desktop. Calibration itself is intentionally drawn
                     * directly against the physical raster elsewhere.
                     */
                    if (desktop_geometry_locked &&
                        region_fits_u32(
                            desktop_output_x,
                            desktop_output_y,
                            desktop_output_width,
                            desktop_output_height,
                            OUTPUT_WIDTH,
                            OUTPUT_HEIGHT)) {

                        safe_x0 = (float)desktop_output_x;
                        safe_y0 = (float)desktop_output_y;
                        safe_w = (float)desktop_output_width;
                        safe_h = (float)desktop_output_height;
                    }

                    scale_x =
                        (safe_w - 12.0f) /
                        (float)OSK_WIDTH;

                    scale_y =
                        (safe_h - 12.0f) /
                        (float)OSK_HEIGHT;

                    scale =
                        (scale_x < scale_y) ?
                        scale_x : scale_y;

                    /*
                     * Menus can grow to use the safe desktop.
                     * The normal keyboard remains 1:1 unless it must shrink.
                     */
                    if (!(display_menu_visible ||
                          system_menu_visible ||
                          display_confirm_visible ||
                          display_restore_ack_visible) &&
                        scale > 1.0f) {

                        scale = 1.0f;
                    }

                    if (scale < 0.25f)
                        scale = 0.25f;

                    dst_w = (float)OSK_WIDTH * scale;
                    dst_h = (float)OSK_HEIGHT * scale;

                    dst_x0 =
                        safe_x0 +
                        (safe_w - dst_w) / 2.0f;

                    if (display_menu_visible ||
                        system_menu_visible ||
                        display_confirm_visible ||
                        display_restore_ack_visible) {

                        dst_y0 =
                            safe_y0 +
                            (safe_h - dst_h) / 2.0f;

                    } else {

                        dst_y0 =
                            safe_y0 +
                            safe_h -
                            dst_h -
                            6.0f;
                    }

                    dst_x1 = dst_x0 + dst_w;
                    dst_y1 = dst_y0 + dst_h;
                }

                gsKit_prim_sprite_texture(
                    gsGlobal,
                    &osk_tex,
                    ps2vnc_display_output_x(dst_x0),
                    ps2vnc_display_output_y(dst_y0),
                    0.0f, 0.0f,
                    ps2vnc_display_output_x(dst_x1),
                    ps2vnc_display_output_y(dst_y1),
                    (float)OSK_WIDTH,
                    (float)OSK_HEIGHT,
                    2,
                    tex_color
                );
            }
        } else {
            rendered_osk_generation = osk_generation;
        }

        pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_POST_DRAW);

        /*
         * TEST13E HIRES presentation.
         *
         * Preserve the logical DRAW -> FLIP ordering, but HIRES performs
         * the actual rendering in scanline-driven passes.
         *
         * Legacy debug stage IDs are retained intentionally.
         */
        pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_PRE_QUEUE);
        {
            u64 t0 = GetTimerSystemTime();

            if (ps2vnc_display_finish_draw(gsGlobal) < 0)
                return -1;

            profile_present_sync_ticks += GetTimerSystemTime() - t0;
        }
        pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_POST_QUEUE);

        pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_PRE_FLIP);
        {
            u64 t0 = GetTimerSystemTime();

            if (ps2vnc_display_flip(gsGlobal) < 0)
                return -1;

            profile_flip_ticks += GetTimerSystemTime() - t0;
        }
        pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_POST_FLIP);

        /*
         * After the initial complete frame, request only pixels that changed.
         * Queued controller traffic is flushed before this main-thread request.
         */
        if (controller_thread_error) {
            pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_CONTROLLER_ERROR);
            live_error_code = 201;
            debug_emit_now();
            screenf("LIVE ERR 201 controller");
            return -1;
        }

        {
            u64 t0 = GetTimerSystemTime();
            if (ps2vnc_display_wait_presentation_memory_idle(
                    gsGlobal) < 0)
                return -1;

            profile_dirty_wait_ticks += GetTimerSystemTime() - t0;
        }

        pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_PRE_REQUEST);
        profile_rfb_start_tick = GetTimerSystemTime();
        /*
         * TEST15D3-A3: both presentation buffers are kept coherent after
         * every completed update, so normal live traffic can be incremental.
         */
        if (!rfb_update_request_outstanding) {
        if (request_framebuffer(sock, 1) < 0) {
            int recovered_sock;

            /*
             * An unconfirmed display owns transport failure as part of its
             * transaction. Abandon the candidate instead of generically
             * recovering into an unapproved display configuration.
             *
             * KEEP is excluded: once explicitly chosen, an unrelated RFB
             * failure no longer rejects the selected display mode.
             */
            if (display_confirm_pending &&
                display_confirm_action != DISPLAY_CONFIRM_ACTION_KEEP) {

                display_confirm_action =
                    DISPLAY_CONFIRM_ACTION_ROLLBACK;

                recovered_sock =
                    ps2vnc_global_rollback_display_transaction(
                        sock,
                        1,
                    rfb_framebuffer,
                        gs_framebuffer,
                        &gsGlobal,
                        &tex,
                        &osk_tex);

                if (recovered_sock < 0)
                    return -1;

                sock = recovered_sock;

                display_confirm_return_to_active_menu();
                rendered_osk_generation = osk_generation - 1;
                continue;
            }

            pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_REQUEST_ERROR);
            live_error_code = 202;
            debug_emit_now();
            screenf("LIVE ERR 202 request");

            recovered_sock =
                recover_rfb_connection(
                    sock,
                    rfb_framebuffer,
                    gs_framebuffer,
                    gsGlobal,
                    &tex,
                    &osk_tex
                );

            if (recovered_sock < 0) {
                if (exit_requested)
                    return 1;

                return -1;
            }

            sock = recovered_sock;

            /*
             * Recovery status temporarily reused the OSK bitmap.
             * Force a fresh keyboard render if the OSK was open.
             */
            osk_generation++;
            rendered_osk_generation =
                osk_generation - 1;

            continue;
        }

            rfb_update_request_outstanding = 1;
        }

        if (ps2vnc_display_begin_update() < 0)
            return -1;

        pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_POST_REQUEST);

        {
            int changed_bytes;
            int direct_present_complete;

            pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_WAIT_RFB);

            /*
             * Snapshot the already-rendered confirmation generation.
             * Any later controller-side countdown/selection generation
             * change may yield at the NEXT server-message boundary.
             */
            rfb_confirm_wait_generation =
                rendered_osk_generation;

            rfb_confirm_live_yield_enabled =
                (display_confirm_pending &&
                 display_confirm_visible) ? 1 : 0;

            changed_bytes = receive_framebuffer_update(
                sock,
                rfb_framebuffer,
                gs_framebuffer,
                0,
                0
            );

            rfb_confirm_live_yield_enabled = 0;

            if (changed_bytes == RFB_RECEIVE_LOCAL_REDRAW) {
                /*
                 * No server-message byte was consumed, so there is no
                 * partial decoder state to finish. The original incremental
                 * request remains outstanding while the outer loop gets a
                 * chance to draw/flip the new confirmation bitmap.
                 */
                ps2vnc_display_direct_write_cancel();
                continue;
            }

            /*
             * Any real FramebufferUpdate completion or transport failure
             * consumes/invalidates the outstanding request contract.
             */
            rfb_update_request_outstanding = 0;

            direct_present_complete =
                ps2vnc_display_finish_update_decode();

            if (changed_bytes >= 0) {
                profile_rfb_ticks +=
                    GetTimerSystemTime() - profile_rfb_start_tick;
                profile_rfb_updates++;
                if (changed_bytes > 0)
                    profile_changed_bytes += (unsigned int)changed_bytes;
            }

            debug_changed_bytes = changed_bytes;
            pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_POST_RECEIVE);

            if (changed_bytes < 0) {
                /*
                 * recv_exact() deliberately aborts when blind-X/timeout
                 * publishes ROLLBACK. A genuine receive failure while the
                 * candidate is still awaiting confirmation has the same
                 * transaction semantics: restore known-good, not candidate.
                 */
                if (display_confirm_pending &&
                    display_confirm_action !=
                        DISPLAY_CONFIRM_ACTION_KEEP) {

                    int rollback_sock;

                    display_confirm_action =
                        DISPLAY_CONFIRM_ACTION_ROLLBACK;

                    rollback_sock =
                        ps2vnc_global_rollback_display_transaction(
                            sock,
                            1,
                    rfb_framebuffer,
                            gs_framebuffer,
                            &gsGlobal,
                            &tex,
                            &osk_tex);

                    if (rollback_sock < 0)
                        return -1;

                    sock = rollback_sock;

                    display_confirm_return_to_active_menu();
                    rendered_osk_generation = osk_generation - 1;
                    continue;
                }

                if (runtime_action_request ==
                    RUNTIME_ACTION_REFRESH_RFB) {

                    int recovered_sock =
                        perform_manual_rfb_recovery(
                            sock,
                            rfb_framebuffer,
                            gs_framebuffer,
                            gsGlobal,
                            &tex,
                            &osk_tex
                        );

                    if (recovered_sock < 0) {
                        if (exit_requested)
                            return 1;

                        return -1;
                    }

                    sock = recovered_sock;

                    rendered_osk_generation =
                        osk_generation - 1;

                    continue;
                }

                if (exit_requested)
                    return 1;

                pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_RECEIVE_ERROR);
                debug_emit_now();
                screenf("LIVE ERR %d type=%u",
                        live_error_code,
                        live_message_type);
                screenf("rect %u/%u enc=%08x",
                        live_rect_index,
                        live_rect_count,
                        live_encoding);
                screenf("xywh %u,%u %ux%u",
                        live_rect_x,
                        live_rect_y,
                        live_rect_w,
                        live_rect_h);
                screenf("hdr %02x%02x%02x%02x %02x%02x%02x%02x",
                        live_hdr_raw[0],
                        live_hdr_raw[1],
                        live_hdr_raw[2],
                        live_hdr_raw[3],
                        live_hdr_raw[4],
                        live_hdr_raw[5],
                        live_hdr_raw[6],
                        live_hdr_raw[7]);
                screenf("    %02x%02x%02x%02x hav=%u rn=%d",
                        live_hdr_raw[8],
                        live_hdr_raw[9],
                        live_hdr_raw[10],
                        live_hdr_raw[11],
                        live_hdr_start_avail,
                        live_hdr_recv_n);
                screenf("upd %02x%02x%02x%02x",
                        live_update_raw[0],
                        live_update_raw[1],
                        live_update_raw[2],
                        live_update_raw[3]);

                {
                    int recovered_sock =
                        recover_rfb_connection(
                            sock,
                            rfb_framebuffer,
                            gs_framebuffer,
                            gsGlobal,
                            &tex,
                            &osk_tex
                        );

                    if (recovered_sock < 0) {
                        if (exit_requested)
                            return 1;

                        return -1;
                    }

                    sock = recovered_sock;

                    osk_generation++;
                    rendered_osk_generation =
                        osk_generation - 1;

                    continue;
                }
            }

            /*
             * The RFB stream is already incremental. The active presentation
             * backend owns how this successfully completed changed update
             * becomes visible.
             */
            if (changed_bytes > 0 && !TEST29_DISCARD_LIVE_PIXELS) {
                pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_PRE_TEXTURE);

                if (ps2vnc_display_commit_update(
                        gsGlobal,
                        &tex,
                        direct_present_complete) < 0)
                    return -1;

                pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_POST_TEXTURE);
            }

        }
    }
}

int main(int argc, char *argv[])
{
    struct ip4_addr IP, NM, GW;
    int sock;
    unsigned short *rfb_framebuffer;
    unsigned short *gs_framebuffer;

    /*
     * TEST15B2E:
     * RFB is already requested in GS-compatible B5:G5:R5 ordering.
     * Use one authoritative EE framebuffer for both receive and GS-ready
     * storage instead of writing every changed pixel through two buffers.
     *
     * Keep the rfb_framebuffer alias temporarily so the proven connection,
     * resize and recovery plumbing remains unchanged for this experiment.
     */
    gs_framebuffer = (unsigned short *)memalign(
        128,
        VNC_WIDTH * VNC_HEIGHT * sizeof(unsigned short)
    );

    if (gs_framebuffer == NULL)
        return 1;

    rfb_framebuffer = gs_framebuffer;

    memset(gs_framebuffer, 0,
           VNC_WIDTH * VNC_HEIGHT * sizeof(unsigned short));

    /*
     * Keep the pre-screen startup unchanged; once debug output is available,
     * every subsequent potentially blocking step has a short marker.
     */
    sceSifInitRpc(0);
    while (!SifIopReset("", 0)) {}
    while (!SifIopSync()) {}

    sceSifInitRpc(0);
    SifLoadFileInit();
    SifInitIopHeap();
    sbv_patch_enable_lmb();

    SifExecModuleBuffer(DEV9_irx, size_DEV9_irx, 0, NULL, NULL);
    SifExecModuleBuffer(NETMAN_irx, size_NETMAN_irx, 0, NULL, NULL);
    SifExecModuleBuffer(SMAP_irx, size_SMAP_irx, 0, NULL, NULL);

    NetManInit();

    init_scr();
    scr_setCursor(0);
    screen_row = 0;

    screenf("TEST31 RX SENTINEL TRACE");
    screenf("Hold L1 + R1 to force display calibration");

    screenf("PAD MOD 1");
    if (SifLoadModule("rom0:XSIO2MAN", 0, NULL) < 0) {
        screenf("ERR XSIO2");
        goto wait;
    }

    screenf("PAD MOD 2");
    if (SifLoadModule("rom0:XPADMAN", 0, NULL) < 0) {
        screenf("ERR XPAD");
        goto wait;
    }

    screenf("PAD INIT");
    if (padInit(0) != 1) {
        screenf("ERR PADINIT");
        goto wait;
    }

    screenf("PAD OPEN");
    if (padPortOpen(0, 0, pad_area) == 0) {
        screenf("ERR PADOPEN");
        goto wait;
    }

    /*
     * Test11K-B:
     * Force DualShock analog mode so L3/R3 and true analog coordinates
     * are available without requiring the user to press the controller's
     * ANALOG button manually.
     *
     * MMODE_LOCK asks the pad to remain in this mode.
     */
    screenf("PAD ANALOG");

    if (wait_pad_ready_after_mode_change() < 0) {
        screenf("ERR PAD READY");
        goto wait;
    }

    if (padSetMainMode(
            0,
            0,
            PAD_MMODE_DUALSHOCK,
            PAD_MMODE_LOCK) == 0) {
        screenf("ERR PAD MODE");
        goto wait;
    }

    if (wait_pad_ready_after_mode_change() < 0) {
        screenf("ERR PAD ANALOG WAIT");
        goto wait;
    }

    screenf("PAD OK");

    force_screen_fit_calibration =
        startup_force_calibration_requested();

    if (force_screen_fit_calibration)
        screenf("L1+R1: FORCE CALIBRATION");

    IP4_ADDR(&IP, 192, 168, 50, 2);
    IP4_ADDR(&NM, 255, 255, 255, 0);
    IP4_ADDR(&GW, 192, 168, 50, 1);

    screenf("TCP INIT");

    if (ps2ipInit(&IP, &NM, &GW) < 0) {
        screenf("ERROR: ps2ipInit() failed");
        goto wait;
    }

    screenf("Waiting for Ethernet link...");

    if (ethWaitValidNetIFLinkState() != 0) {
        screenf("Unable to establish Ethernet connection");
        screenf("Returning to system menu...");
        goto exit_to_osdsys;
    }

    screenf("Ethernet link is UP");

    /*
     * TEST13J-D4-A3:
     * Retrieve the general PS2VNC configuration before opening RFB.
     *
     * Failure is intentionally non-fatal.  Without a valid stored display
     * preset, the existing interactive calibration path remains authoritative.
     */
    ps2vnc_config_load_from_pi();

    /*
     * H4C3B2B:
     * Durable display authority must be known before ordinary RFB startup.
     * A pending transaction may imply that TigerVNC is still at a geometry
     * incompatible with the confirmed startup profile.
     */
    if (ps2vnc_display_transaction_startup_preflight() < 0) {
        screenf("DISPLAY TX: unsafe startup blocked");
        screenf("Returning to system menu...");
        goto exit_to_osdsys;
    }

    if (ps2vnc_startup_prepare_display_transaction() < 0) {
        screenf("DISPLAY TX: startup recovery preparation failed");
        screenf("Returning to system menu...");
        goto exit_to_osdsys;
    }

    if (ps2vnc_startup_apply_authoritative_profile() < 0) {
        screenf("DISPLAY TX: startup profile apply failed");
        screenf("Returning to system menu...");
        goto exit_to_osdsys;
    }

    pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_NET_READY);
    if (pstvnc_debug_init() < 0)
        screenf("WARN: debug UDP did not initialize");
    else
        debug_emit_now();

    rfb_rx_pos = 0;
    rfb_rx_end = 0;
    rfb_rx_syscalls = 0;
    rfb_last_recv_n = 0;
    rfb_last_recv_call = 0;
    memset((void *)live_hdr_raw, 0, sizeof(live_hdr_raw));
    memset((void *)live_update_raw, 0, sizeof(live_update_raw));

    screenf("Establishing VNC session...");

    sock = ps2vnc_startup_connect_reconciled(
        rfb_framebuffer,
        gs_framebuffer
    );

    if (sock < 0) {
        screenf("Unable to establish VNC session");
        screenf("Returning to system menu...");
        goto exit_to_osdsys;
    }

    screenf("FULL FRAMEBUFFER RECEIVED");
    screenf("Starting incremental gsKit display...");

    live_error_code = 0;
    live_message_type = 0;
    live_rect_count = 0;
    live_rect_index = 0;
    live_rect_x = 0;
    live_rect_y = 0;
    live_rect_w = 0;
    live_rect_h = 0;
    live_encoding = 0;

    {
        int live_result = display_live_desktop(
            sock,
            rfb_framebuffer,
            gs_framebuffer);

        if (live_result > 0)
            goto exit_to_osdsys;

        if (live_result < 0)
            goto exit_to_osdsys;
    }

exit_to_osdsys:
    screenf("Returning to system menu...");
    LoadExecPS2("rom0:OSDSYS", 0, NULL);

    /*
     * LoadExecPS2 normally never returns.  If it does, fall back to the
     * existing stopped state so we get visible evidence instead of hanging
     * somewhere unknown.
     */
    screenf("ERROR: OSDSYS return failed");

wait:
    screenf("Stopped - reset PS2 when done");
    SleepThread();

    return 0;
}
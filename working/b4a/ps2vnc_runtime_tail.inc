 void debug_emit_now(void)
{
    pstvnc_diagnostics_debug_report_t report;
    unsigned int i;

    if (!pstvnc_debug_is_ready())
        return;

    report.frame = debug_frame_counter;
    report.stage = pstvnc_debug_get_stage();
    report.changed_bytes = debug_changed_bytes;

    report.message_type = live_message_type;
    report.rect_index = live_rect_index;
    report.rect_count = live_rect_count;
    report.rect_x = live_rect_x;
    report.rect_y = live_rect_y;
    report.rect_w = live_rect_w;
    report.rect_h = live_rect_h;
    report.encoding = live_encoding;

    report.controller_loop = controller_loop_counter;
    report.controller_error = controller_thread_error;
    report.osk_visible = osk_visible;

    for (i = 0; i < 12; ++i)
        report.header_raw[i] = live_hdr_raw[i];

    report.header_start_pos = live_hdr_start_pos;
    report.header_start_end = live_hdr_start_end;
    report.header_start_avail = live_hdr_start_avail;
    report.header_end_pos = live_hdr_end_pos;
    report.header_end_end = live_hdr_end_end;
    report.header_recv_n = live_hdr_recv_n;
    report.header_recv_call = live_hdr_recv_call;

    report.hotkey_mask = debug_hotkey_mask;
    report.hotkey_stable_polls = debug_hotkey_stable_polls;
    report.hotkey_settle_target = debug_hotkey_settle_target;
    report.hotkey_action = debug_hotkey_action;
    report.hotkey_trigger = debug_hotkey_trigger;
    report.hotkey_hold_polls = debug_hotkey_hold_polls;
    report.system_menu_countdown_seconds =
        system_menu_countdown_seconds;
    report.hotkey_latched = debug_hotkey_latched;
    report.hotkey_quarantine = debug_hotkey_quarantine;

    report.system_menu_visible = system_menu_visible;
    report.display_menu_visible = display_menu_visible;
    report.runtime_action_request = runtime_action_request;

    report.transition_phase = debug_transition_phase;
    report.controller_pad_pause_requested =
        controller_pad_pause_requested;
    report.controller_pad_pause_ack =
        controller_pad_pause_ack;

    report.rfb_out_count = rfb_out_count;

    pstvnc_diagnostics_report_debug(&report);
}


unsigned int profile_ticks_us(u64 ticks)
{
    u32 sec = 0;
    u32 usec = 0;

    TimerBusClock2USec(ticks, &sec, &usec);
    return sec * 1000000u + usec;
}


/*
 * D17AL-F8I1B:
 *
 * The System menu is global over ORDINARY local UI, but it must never
 * punch through a provisional display transaction or libpad calibration
 * ownership boundary.
 *
 * Pending mode/calibration requests are included so there is no race
 * between controller publication and main-thread transaction ownership.
 */
 int ps2vnc_system_overlay_allowed(void)
{
    if (display_restore_ack_visible ||
        display_confirm_pending ||
        display_transition_switch_in_progress ||
        display_mode_request != DISPLAY_MODE_REQUEST_NONE ||
        display_calibration_request != DISPLAY_MODE_REQUEST_NONE ||
        controller_pad_pause_requested)
        return 0;

    return 1;
}


/*
 * Return nonzero only when COOLDOWN actually expires.
 */
 int ps2vnc_system_refresh_update_state(void)
{
    unsigned int elapsed_us;

    if (system_refresh_state != SYSTEM_REFRESH_COOLDOWN)
        return 0;

    elapsed_us =
        profile_ticks_us(
            GetTimerSystemTime() -
            system_refresh_cooldown_started_tick);

    if (elapsed_us <
        SYSTEM_REFRESH_COOLDOWN_SECONDS * 1000000u)
        return 0;

    system_refresh_state = SYSTEM_REFRESH_READY;
    system_refresh_cooldown_started_tick = 0;
    system_menu_status = SYSTEM_MENU_STATUS_NONE;

    if (system_menu_visible)
        osk_generation++;

    return 1;
}


/*
 * Sole publication gate for an explicit manual Refresh request.
 */
 int ps2vnc_system_refresh_accept_request(void)
{
    (void)ps2vnc_system_refresh_update_state();

    if (!ps2vnc_system_overlay_allowed())
        return 0;

    if (system_refresh_state != SYSTEM_REFRESH_READY)
        return 0;

    if (runtime_action_request != RUNTIME_ACTION_NONE)
        return 0;

    system_refresh_state = SYSTEM_REFRESH_REFRESHING;
    system_refresh_cooldown_started_tick = 0;

    system_menu_status = SYSTEM_MENU_STATUS_REFRESHING;
    system_menu_selection = SYSTEM_MENU_REFRESH;

    runtime_action_request = RUNTIME_ACTION_REFRESH_RFB;

    osk_generation++;
    return 1;
}


/*
 * Exit remains available during ordinary Refresh/recovery, but never cuts
 * across a provisional display transaction.
 */
 int ps2vnc_system_exit_accept_request(void)
{
    if (!ps2vnc_system_overlay_allowed())
        return 0;

    runtime_action_request =
        RUNTIME_ACTION_EXIT_TO_SYSTEM_MENU;

    exit_requested = 1;
    return 1;
}


 void display_confirm_arm_transaction(
    const ps2vnc_video_mode_t *previous_mode,
    unsigned int previous_x,
    unsigned int previous_y,
    unsigned int previous_w,
    unsigned int previous_h,
    int previous_screen_fit_width,
    int previous_screen_fit_height,
    int previous_screen_fit_offset_x,
    int previous_screen_fit_offset_y,
    int previous_screen_fit_full_bypass)
{
    display_confirm_previous_mode = previous_mode;
    display_confirm_previous_backend = active_display_backend;
    display_confirm_previous_x = previous_x;
    display_confirm_previous_y = previous_y;
    display_confirm_previous_w = previous_w;
    display_confirm_previous_h = previous_h;
    display_confirm_previous_desktop_w = desktop_width;
    display_confirm_previous_desktop_h = desktop_height;

    display_confirm_previous_screen_fit_width =
        previous_screen_fit_width;
    display_confirm_previous_screen_fit_height =
        previous_screen_fit_height;
    display_confirm_previous_screen_fit_offset_x =
        previous_screen_fit_offset_x;
    display_confirm_previous_screen_fit_offset_y =
        previous_screen_fit_offset_y;
    display_confirm_previous_screen_fit_full_bypass =
        previous_screen_fit_full_bypass;

    display_confirm_selection = DISPLAY_CONFIRM_GO_BACK;
    display_confirm_seconds = DISPLAY_CONFIRM_TIMEOUT_SECONDS;
    display_confirm_action = DISPLAY_CONFIRM_ACTION_NONE;
    display_confirm_visible = 0;
    display_confirm_started_tick = GetTimerSystemTime();

    /* Publish pending last: controller may observe it immediately. */
    display_confirm_pending = 1;
}


 void display_confirm_disarm_transaction(void)
{
    /* Withdraw ownership first; remaining fields are then private cleanup. */
    display_confirm_pending = 0;
    display_confirm_visible = 0;
    display_confirm_seconds = 0;
    display_confirm_action = DISPLAY_CONFIRM_ACTION_NONE;
    display_confirm_previous_mode = NULL;
    display_confirm_previous_backend = PS2VNC_BACKEND_NONE;
    display_confirm_previous_x = 0;
    display_confirm_previous_y = 0;
    display_confirm_previous_w = 0;
    display_confirm_previous_h = 0;
    display_confirm_previous_desktop_w = 0;
    display_confirm_previous_desktop_h = 0;

    display_confirm_previous_screen_fit_width = 0;
    display_confirm_previous_screen_fit_height = 0;
    display_confirm_previous_screen_fit_offset_x = 0;
    display_confirm_previous_screen_fit_offset_y = 0;
    display_confirm_previous_screen_fit_full_bypass = 0;

    display_confirm_started_tick = 0;
}



 void display_confirm_return_to_active_menu(void)
{
    /*
     * F8J1C:
     * Remote test restoration is automatically acknowledged.  Do not
     * synthesize a Display Modes UI that was never open before the test.
     *
     * All ordinary human KEEP/Go Back/recovery transactions retain their
     * historical return-to-active-row behavior.
     */
    if (remote_display_suppress_next_active_menu_return) {
        remote_display_suppress_next_active_menu_return = 0;
        return;
    }

    /*
     * H4B2C:
     * Never expose Display Settings until durable restore ACK ownership
     * has completed.
     */
    if (display_restore_ack_visible)
        return;

    display_menu_open_for_active(0);
}



 unsigned int display_confirm_remaining_seconds(void)
{
    unsigned int elapsed_us;

    if (!display_confirm_pending)
        return 0;

    elapsed_us = profile_ticks_us(
        GetTimerSystemTime() - display_confirm_started_tick);

    if (elapsed_us >=
        DISPLAY_CONFIRM_TIMEOUT_SECONDS * 1000000u)
        return 0;

    return DISPLAY_CONFIRM_TIMEOUT_SECONDS -
        (elapsed_us / 1000000u);
}

 void profile_emit_now(void)
{
    pstvnc_diagnostics_profile_report_t report;

    report.frame = debug_frame_counter;
    report.rfb_updates = profile_rfb_updates;
    report.dirty_updates = profile_dirty_updates;
    report.changed_bytes = profile_changed_bytes;
    report.raw_rects = profile_raw_rects;
    report.hextile_rects = profile_hextile_rects;

    report.present_sync_us =
        profile_ticks_us(profile_present_sync_ticks);
    report.flip_us =
        profile_ticks_us(profile_flip_ticks);
    report.rfb_us =
        profile_ticks_us(profile_rfb_ticks);
    report.rx_wait_us =
        profile_ticks_us(profile_rx_wait_ticks);
    report.hextile_us =
        profile_ticks_us(profile_hextile_ticks);
    report.hextile_rx_wait_us =
        profile_ticks_us(profile_hextile_rx_wait_ticks);
    report.dirty_wait_us =
        profile_ticks_us(profile_dirty_wait_ticks);
    report.dirty_work_us =
        profile_ticks_us(profile_dirty_work_ticks);

    pstvnc_diagnostics_report_profile(&report);

    profile_present_sync_ticks = 0;
    profile_flip_ticks = 0;
    profile_rfb_ticks = 0;
    profile_rx_wait_ticks = 0;
    profile_hextile_ticks = 0;
    profile_hextile_rx_wait_ticks = 0;
    profile_dirty_wait_ticks = 0;
    profile_dirty_work_ticks = 0;
    profile_rfb_updates = 0;
    profile_dirty_updates = 0;
    profile_changed_bytes = 0;
    profile_raw_rects = 0;
    profile_hextile_rects = 0;
}


 void debug_emit_idle_poll(void)
{
    debug_idle_poll_counter++;

    if ((debug_idle_poll_counter % 250) == 0)
        debug_emit_now();
}

/*
 * Wait for libpad to finish an asynchronous mode change.
 *
 * The PS2SDK pad sample waits for STABLE/FINDCTP1 after
 * padSetMainMode().  Give this controller up to ~2 seconds.
 */
 int wait_pad_ready_after_mode_change(void)
{
    const struct timespec delay = { 0, 10000000 }; /* 10 ms */
    int i;

    for (i = 0; i < 200; i++) {
        int state = padGetState(0, 0);

        if (state == PAD_STATE_STABLE ||
            state == PAD_STATE_FINDCTP1)
            return 0;

        nanosleep(&delay, NULL);
    }

    return -1;
}


/*
 * Startup-only display recalibration chord.
 *
 * The normal libpad button bits are active-low, matching the controller
 * thread's existing 0xFFFF ^ buttons.btns decoding.
 *
 * Normal startup returns immediately when L1+R1 are not held.  If both
 * buttons are already down, require about 300 ms of continuous hold before
 * accepting the override.
 */
 int startup_force_calibration_requested(void)
{
    const struct timespec delay = { 0, 10000000 }; /* 10 ms */
    struct padButtonStatus buttons;
    unsigned int pressed;
    int consecutive = 0;
    int saw_valid_sample = 0;
    int i;

    for (i = 0; i < 50; i++) {

        if (padGetState(0, 0) == PAD_STATE_STABLE &&
            padRead(0, 0, &buttons) != 0) {

            pressed = 0xFFFFu ^ (unsigned int)buttons.btns;

            if (!saw_valid_sample) {
                saw_valid_sample = 1;

                if ((pressed & (PAD_L1 | PAD_R1)) !=
                    (PAD_L1 | PAD_R1))
                    return 0;
            }

            if ((pressed & (PAD_L1 | PAD_R1)) ==
                (PAD_L1 | PAD_R1)) {

                consecutive++;

                if (consecutive >= 30)
                    return 1;

            } else {
                return 0;
            }
        }

        nanosleep(&delay, NULL);
    }

    return 0;
}


static void EthStatusCheckCb(s32 alarm_id, u16 time, void *common)
{
    iWakeupThread(*(int *)common);
}

static int WaitValidNetState(int (*checkingFunction)(void))
{
    int thread_id;
    int retry_cycles;

    thread_id = GetThreadId();

    for (retry_cycles = 0; checkingFunction() == 0; retry_cycles++) {
        SetAlarm(1000 * 16, &EthStatusCheckCb, &thread_id);
        SleepThread();

        if (retry_cycles >= 9)
            return -1;
    }

    return 0;
}

int ethGetNetIFLinkStatus(void)
{
    return NetManIoctl(
        NETMAN_NETIF_IOCTL_GET_LINK_STATUS,
        NULL, 0, NULL, 0) == NETMAN_NETIF_ETH_LINK_STATE_UP;
}

 int ethWaitValidNetIFLinkState(void)
{
    return WaitValidNetState(&ethGetNetIFLinkStatus);
}

int rfb_connect_and_handshake_limited(
    unsigned int max_width,
    unsigned int max_height)
{
    static const char client_version[12] = "RFB 003.008\n";

    int sock = -1;
    int i;
    int none_found = 0;
    struct sockaddr_in server;

    char server_version[13];
    unsigned char security_count;
    unsigned char security_types[255];
    unsigned char security_choice = 1;
    unsigned char security_result[4];
    unsigned char shared_flag = 1;
    unsigned char server_init[24];

    unsigned int width;
    unsigned int height;
    unsigned int name_len;
    unsigned int name_take;
    unsigned int remaining;

    char desktop_name[128];
    unsigned char discard[64];

    screenf(
        "Connecting to %s:%d...",
        PS2VNC_BOOTSTRAP_PI_IP,
        VNC_PORT
    );

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        screenf("ERROR: socket() failed: %d", sock);
        goto fail;
    }

    memset(&server, 0, sizeof(server));
    server.sin_len = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_port = htons(VNC_PORT);
    server.sin_addr.s_addr =
        inet_addr(PS2VNC_BOOTSTRAP_PI_IP);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        screenf("ERROR: connect() failed");
        goto fail;
    }

    screenf("TCP connected");

    if (recv_exact(sock, server_version, 12) != 12) {
        screenf("ERROR: could not read RFB version");
        goto fail;
    }

    server_version[12] = '\0';

    if (server_version[11] == '\n')
        server_version[11] = '\0';

    sanitize_text(server_version);
    screenf("Server version: %s", server_version);

    if (send_exact(sock, client_version, 12) != 12) {
        screenf("ERROR: could not send RFB version");
        goto fail;
    }

    if (recv_exact(sock, &security_count, 1) != 1) {
        screenf("ERROR: no security-type count");
        goto fail;
    }

    if (security_count == 0) {
        unsigned char lenbuf[4];
        unsigned int reason_len;
        unsigned int take;
        char reason[128];

        if (recv_exact(sock, lenbuf, 4) != 4) {
            screenf("ERROR: server rejected connection");
            goto fail;
        }

        reason_len = read_be32(lenbuf);
        take = reason_len;

        if (take > sizeof(reason) - 1)
            take = sizeof(reason) - 1;

        if (take > 0 && recv_exact(sock, reason, take) != (int)take) {
            screenf("ERROR: failed reading rejection");
            goto fail;
        }

        reason[take] = '\0';
        sanitize_text(reason);

        remaining = reason_len - take;

        while (remaining > 0) {
            unsigned int chunk = remaining;

            if (chunk > sizeof(discard))
                chunk = sizeof(discard);

            if (recv_exact(sock, discard, chunk) != (int)chunk)
                break;

            remaining -= chunk;
        }

        screenf("SERVER REJECTED: %s", reason);
        goto fail;
    }

    if (recv_exact(sock, security_types, security_count) != security_count) {
        screenf("ERROR: failed reading security types");
        goto fail;
    }

    for (i = 0; i < security_count; i++) {
        if (security_types[i] == 1) {
            none_found = 1;
            break;
        }
    }

    if (!none_found) {
        screenf("ERROR: SecurityType None not offered");
        goto fail;
    }

    if (send_exact(sock, &security_choice, 1) != 1) {
        screenf("ERROR: could not select security type");
        goto fail;
    }

    if (recv_exact(sock, security_result, 4) != 4) {
        screenf("ERROR: no SecurityResult");
        goto fail;
    }

    if (read_be32(security_result) != 0) {
        screenf("ERROR: authentication failed");
        goto fail;
    }

    if (send_exact(sock, &shared_flag, 1) != 1) {
        screenf("ERROR: could not send ClientInit");
        goto fail;
    }

    if (recv_exact(sock, server_init, sizeof(server_init)) != sizeof(server_init)) {
        screenf("ERROR: failed reading ServerInit");
        goto fail;
    }

    width = read_be16(&server_init[0]);
    height = read_be16(&server_init[2]);
    name_len = read_be32(&server_init[20]);

    screenf("Handshake OK - desktop %ux%u", width, height);

    /*
     * H4C3C2A:
     * The caller explicitly owns the ServerInit acceptance envelope.
     *
     * Ordinary callers still use OUTPUT_WIDTH/OUTPUT_HEIGHT through the
     * wrapper below. Startup reconciliation will later be allowed to use
     * the larger allocated RFB framebuffer capacity long enough to repair
     * a stale TigerVNC geometry.
     *
     * Never permit an envelope larger than PS2VNC's backing framebuffer.
     */
    if (max_width == 0 ||
        max_height == 0 ||
        max_width > VNC_WIDTH ||
        max_height > VNC_HEIGHT) {

        screenf(
            "ERROR: invalid RFB limit %ux%u",
            max_width,
            max_height
        );
        goto fail;
    }

    if (width == 0 || height == 0 ||
        width > max_width ||
        height > max_height) {

        screenf(
            "ERROR: unsupported VNC desktop %ux%u",
            width,
            height
        );
        goto fail;
    }

    if (desktop_geometry_locked) {
        if (width != desktop_width ||
            height != desktop_height) {

            screenf(
                "ERROR: expected %ux%u VNC desktop",
                desktop_width,
                desktop_height
            );
            goto fail;
        }
    } else {
        /*
         * A prior interrupted resize may have left TigerVNC smaller than
         * 1280x720. Treat the server's actual geometry as authoritative
         * until calibration locks the final safe desktop.
         */
        desktop_width = width;
        desktop_height = height;
    }

    name_take = name_len;

    if (name_take > sizeof(desktop_name) - 1)
        name_take = sizeof(desktop_name) - 1;

    if (name_take > 0) {
        if (recv_exact(sock, desktop_name, name_take) != (int)name_take) {
            screenf("ERROR: failed reading desktop name");
            goto fail;
        }
    }

    desktop_name[name_take] = '\0';
    sanitize_text(desktop_name);

    remaining = name_len - name_take;

    while (remaining > 0) {
        unsigned int chunk = remaining;

        if (chunk > sizeof(discard))
            chunk = sizeof(discard);

        if (recv_exact(sock, discard, chunk) != (int)chunk) {
            screenf("ERROR: failed discarding long name");
            goto fail;
        }

        remaining -= chunk;
    }

    screenf("Desktop name: %.55s", desktop_name);

    return sock;

fail:
    if (sock >= 0)
        close(sock);

    return -1;
}


/*
 * Preserve the historical handshake contract for every ordinary caller.
 *
 * H4C reconciliation must opt into a wider acceptance envelope explicitly;
 * no existing startup/recovery path is silently weakened.
 */
int rfb_connect_and_handshake(void)
{
    return rfb_connect_and_handshake_limited(
        OUTPUT_WIDTH,
        OUTPUT_HEIGHT);
}


int set_gs555_pixel_format(int sock)
{
    static const unsigned char msg[20] = {
        0, 0, 0, 0,
        16, 15, 0, 1,
        0, 31,
        0, 31,
        0, 31,
        0, 5, 10,
        0, 0, 0
    };

    return send_exact(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}

#define RFB_ENCODING_RAW                    0x00000000u
#define RFB_ENCODING_HEXTILE                0x00000005u
#define RFB_ENCODING_EXTENDED_DESKTOP_SIZE  0xFFFFFECCu

#define HEXTILE_RAW                  0x01u
#define HEXTILE_BACKGROUND_SPECIFIED 0x02u
#define HEXTILE_FOREGROUND_SPECIFIED 0x04u
#define HEXTILE_ANY_SUBRECTS         0x08u
#define HEXTILE_SUBRECTS_COLORED     0x10u

/*
 * TEST13J-D1:
 * State reported by the server's ExtendedDesktopSize pseudo-rectangle.
 *
 * D1 only proves capability and records the current layout. It does not
 * request a new size yet.
 */
 int rfb_extended_desktop_seen = 0;
 unsigned int rfb_extended_desktop_reason = 0;
 unsigned int rfb_extended_desktop_result = 0;
 unsigned int rfb_extended_desktop_width = 0;
 unsigned int rfb_extended_desktop_height = 0;
 unsigned int rfb_extended_desktop_screen_count = 0;

 unsigned int rfb_first_screen_id = 0;
 unsigned int rfb_first_screen_x = 0;
 unsigned int rfb_first_screen_y = 0;
 unsigned int rfb_first_screen_width = 0;
 unsigned int rfb_first_screen_height = 0;
 unsigned int rfb_first_screen_flags = 0;

/*
 * TEST13J-D2:
 * During a deliberate SetDesktopSize transaction, permit the EDS
 * response to report the requested dimensions even though the Raw
 * framebuffer path is still fixed at 1280x720.
 */
 int rfb_resize_expected = 0;
 unsigned int rfb_resize_expected_width = 0;
 unsigned int rfb_resize_expected_height = 0;


int set_raw_encoding(int sock)
{
    static const unsigned char msg[8] = {
        2, 0,
        0, 1,
        0, 0, 0, 0
    };

    return send_exact(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}



/*
 * TEST15D1:
 * Prefer Hextile for ordinary incremental updates while retaining Raw
 * as the mandatory fallback. Startup, resize and recovery full-frame
 * synchronization remain Raw until explicitly switched to this list.
 */
int set_live_encoding(int sock)
{
    static const unsigned char msg[12] = {
        2, 0,
        0, 2,

        /* Hextile = 5 */
        0x00, 0x00, 0x00, 0x05,

        /* Raw = 0 */
        0x00, 0x00, 0x00, 0x00
    };

    return send_exact(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}


/*
 * Advertise Raw plus ExtendedDesktopSize (-308).
 *
 * Raw remains first because it is still our preferred pixel encoding.
 * ExtendedDesktopSize is only a capability declaration.
 */
int set_raw_extended_desktop_encoding(int sock)
{
    static const unsigned char msg[12] = {
        2, 0,
        0, 2,

        /* Raw = 0 */
        0x00, 0x00, 0x00, 0x00,

        /* ExtendedDesktopSize = -308 = 0xFFFFFECC */
        0xFF, 0xFF, 0xFE, 0xCC
    };

    return send_exact(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}



/*
 * Forward declaration: TEST13J-D2 resize confirmation needs to consume
 * an ExtendedDesktopSize FramebufferUpdate before the receiver's full
 * implementation later in this file.
 */
 int receive_framebuffer_update(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    int verbose,
    int require_full);


/*
 * TEST13J-D2:
 * Send RFB SetDesktopSize (client message type 251).
 *
 * This client uses a single-screen layout. The screen ID and flags are
 * preserved from the server's ExtendedDesktopSize capability response.
 */
static int send_set_desktop_size_and_probe(
    int sock,
    unsigned int width,
    unsigned int height)
{
    unsigned char msg[34];

    memset(msg, 0, sizeof(msg));

    /*
     * Bytes 0..23:
     * RFB SetDesktopSize.
     */
    msg[0] = 251;

    msg[2] = (unsigned char)((width >> 8) & 0xFF);
    msg[3] = (unsigned char)(width & 0xFF);

    msg[4] = (unsigned char)((height >> 8) & 0xFF);
    msg[5] = (unsigned char)(height & 0xFF);

    msg[6] = 1; /* one screen */

    msg[8]  =
        (unsigned char)((rfb_first_screen_id >> 24) & 0xFF);
    msg[9]  =
        (unsigned char)((rfb_first_screen_id >> 16) & 0xFF);
    msg[10] =
        (unsigned char)((rfb_first_screen_id >> 8) & 0xFF);
    msg[11] =
        (unsigned char)(rfb_first_screen_id & 0xFF);

    /* Screen x/y remain 0,0. */

    msg[16] = (unsigned char)((width >> 8) & 0xFF);
    msg[17] = (unsigned char)(width & 0xFF);

    msg[18] = (unsigned char)((height >> 8) & 0xFF);
    msg[19] = (unsigned char)(height & 0xFF);

    msg[20] =
        (unsigned char)((rfb_first_screen_flags >> 24) & 0xFF);
    msg[21] =
        (unsigned char)((rfb_first_screen_flags >> 16) & 0xFF);
    msg[22] =
        (unsigned char)((rfb_first_screen_flags >> 8) & 0xFF);
    msg[23] =
        (unsigned char)(rfb_first_screen_flags & 0xFF);

    /*
     * TEST13J-D2 SINGLE-WRITE PROBE:
     *
     * Bytes 24..33 are a second RFB client message:
     * an empty incremental FramebufferUpdateRequest.
     *
     * TigerVNC therefore receives:
     *
     *   SetDesktopSize
     *   FramebufferUpdateRequest
     *
     * from one PS2 send sequence.
     */
    msg[24] = 3; /* FramebufferUpdateRequest */
    msg[25] = 1; /* incremental */

    /*
     * TEST13J-D2:
     * Request one harmless pixel rather than an empty region.
     *
     * The RFB specification permits an empty request, but this avoids
     * depending on the server waking its update machinery for 0x0.
     */
    msg[26] = 0; /* x = 0 */
    msg[27] = 0;
    msg[28] = 0; /* y = 0 */
    msg[29] = 0;
    msg[30] = 0; /* width = 1 */
    msg[31] = 1;
    msg[32] = 0; /* height = 1 */
    msg[33] = 1;

    return send_exact(sock, msg, sizeof(msg)) ==
           sizeof(msg) ? 0 : -1;
}


int request_and_confirm_desktop_size(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    unsigned int width,
    unsigned int height)
{
    unsigned int response_count;

    /*
     * F8J2-B2:
     *
     * SetDesktopSize completion is not necessarily the first EDS
     * notification TigerVNC emits at the new geometry.
     *
     * Hardware packet evidence showed:
     *
     *   reason=0 result=0 requested-size
     *   reason=0 result=0 requested-size
     *   reason=0 result=0 requested-size
     *   reason=1 result=0 requested-size
     *
     * reason=0 is an informational server-side layout notification.
     * reason=1 is the acknowledgement for THIS client's SetDesktopSize.
     *
     * Keep consuming complete FramebufferUpdate messages until the
     * matching client acknowledgement arrives.  Geometry validation
     * remains owned by receive_framebuffer_update() while
     * rfb_resize_expected is set.
     *
     * Bound the number of complete server updates accepted while waiting.
     * recv_exact() retains the existing exit/rollback/recovery escape
     * behavior inside each message.
     */
    const unsigned int response_limit = 16;

    rfb_resize_expected = 1;
    rfb_resize_expected_width = width;
    rfb_resize_expected_height = height;

    if (send_set_desktop_size_and_probe(
            sock,
            width,
            height) < 0) {

        rfb_resize_expected = 0;

        /* F8J2-B2 diagnostic: SetDesktopSize/probe send failure. */
        live_error_code = 120;

        return -1;
    }

    for (response_count = 0;
         response_count < response_limit;
         response_count++) {

        /*
         * Each receive consumes one complete FramebufferUpdate.
         * Clear the observation fields so an unrelated update cannot
         * inherit a previous EDS notification.
         */
        rfb_extended_desktop_seen = 0;
        rfb_extended_desktop_reason = 0;
        rfb_extended_desktop_result = 0;
        rfb_extended_desktop_width = 0;
        rfb_extended_desktop_height = 0;
        rfb_extended_desktop_screen_count = 0;

        if (receive_framebuffer_update(
                sock,
                rfb_framebuffer,
                gs_framebuffer,
                0,
                0) < 0) {

            rfb_resize_expected = 0;
            return -1;
        }

        /*
         * Ordinary framebuffer traffic may be interleaved while the
         * server completes the resize transaction.  It is not the
         * acknowledgement we are waiting for.
         */
        if (!rfb_extended_desktop_seen)
            continue;

        /*
         * receive_framebuffer_update() already rejects an EDS rectangle
         * whose dimensions differ from rfb_resize_expected_width/height.
         *
         * A same-geometry reason other than 1 is informational.  Continue
         * until the acknowledgement specifically associated with this
         * client's SetDesktopSize arrives.
         */
        if (rfb_extended_desktop_reason != 1)
            continue;

        rfb_resize_expected = 0;

        /*
         * reason=1 identifies our request.  result=0 is success.
         */
        if (rfb_extended_desktop_result != 0) {
            live_error_code = 121;
            return -1;
        }

        if (rfb_extended_desktop_width != width ||
            rfb_extended_desktop_height != height) {

            /*
             * Normally unreachable because the receiver validates this
             * while rfb_resize_expected is active.  Keep an explicit
             * defensive diagnostic at the ownership boundary.
             */
            live_error_code = 118;
            return -1;
        }

        return 0;
    }

    rfb_resize_expected = 0;

    /*
     * We consumed bounded complete server updates without seeing the
     * acknowledgement for our SetDesktopSize.
     */
    live_error_code = 122;

    return -1;
}

 int send_pointer_event(
    int sock,
    unsigned char button_mask,
    unsigned int x,
    unsigned int y)
{
    unsigned char msg[6];

    msg[0] = 5;
    msg[1] = button_mask;
    msg[2] = (unsigned char)((x >> 8) & 0xFF);
    msg[3] = (unsigned char)(x & 0xFF);
    msg[4] = (unsigned char)((y >> 8) & 0xFF);
    msg[5] = (unsigned char)(y & 0xFF);

    return send_rfb_message(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}


/*
 * RFB KeyEvent:
 *   byte 0     = message type 4
 *   byte 1     = down flag (1 press, 0 release)
 *   bytes 2-3  = padding
 *   bytes 4-7  = X11 keysym, big-endian
 *
 * This uses the same serialized sender as PointerEvent and
 * FramebufferUpdateRequest, so keyboard traffic cannot interleave with
 * either of them on the TCP byte stream.
 */
 int send_key_event(
    int sock,
    unsigned char down,
    unsigned int keysym)
{
    unsigned char msg[8];

    msg[0] = 4;
    msg[1] = down ? 1 : 0;
    msg[2] = 0;
    msg[3] = 0;
    msg[4] = (unsigned char)((keysym >> 24) & 0xFF);
    msg[5] = (unsigned char)((keysym >> 16) & 0xFF);
    msg[6] = (unsigned char)((keysym >> 8) & 0xFF);
    msg[7] = (unsigned char)(keysym & 0xFF);

    return send_rfb_message(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}


 int send_key_tap(int sock, unsigned int keysym)
{
    unsigned char msg[16];

    /* Key down. */
    msg[0] = 4;
    msg[1] = 1;
    msg[2] = 0;
    msg[3] = 0;
    msg[4] = (unsigned char)((keysym >> 24) & 0xFF);
    msg[5] = (unsigned char)((keysym >> 16) & 0xFF);
    msg[6] = (unsigned char)((keysym >> 8) & 0xFF);
    msg[7] = (unsigned char)(keysym & 0xFF);

    /* Key up. */
    msg[8]  = 4;
    msg[9]  = 0;
    msg[10] = 0;
    msg[11] = 0;
    msg[12] = msg[4];
    msg[13] = msg[5];
    msg[14] = msg[6];
    msg[15] = msg[7];

    return send_rfb_message(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}


/*
 * Send a key with optional real Ctrl/Alt modifiers as one serialized RFB
 * write.  This keeps the entire shortcut atomic relative to framebuffer
 * requests and mouse events.
 *
 * Shift remains a local OSK layer; it changes the final printable keysym.
 */
 int send_modified_key_tap(
    int sock,
    unsigned int keysym,
    int use_ctrl,
    int use_alt)
{
    unsigned char msg[48];
    int pos = 0;

#define APPEND_KEY_EVENT(DOWN, KEYSYM) do {                       \
        unsigned int _k = (unsigned int)(KEYSYM);                 \
        msg[pos + 0] = 4;                                         \
        msg[pos + 1] = (DOWN) ? 1 : 0;                            \
        msg[pos + 2] = 0;                                         \
        msg[pos + 3] = 0;                                         \
        msg[pos + 4] = (unsigned char)((_k >> 24) & 0xFF);        \
        msg[pos + 5] = (unsigned char)((_k >> 16) & 0xFF);        \
        msg[pos + 6] = (unsigned char)((_k >> 8) & 0xFF);         \
        msg[pos + 7] = (unsigned char)(_k & 0xFF);                \
        pos += 8;                                                  \
    } while (0)

    if (use_ctrl)
        APPEND_KEY_EVENT(1, XK_CONTROL_L);

    if (use_alt)
        APPEND_KEY_EVENT(1, XK_ALT_L);

    APPEND_KEY_EVENT(1, keysym);
    APPEND_KEY_EVENT(0, keysym);

    if (use_alt)
        APPEND_KEY_EVENT(0, XK_ALT_L);

    if (use_ctrl)
        APPEND_KEY_EVENT(0, XK_CONTROL_L);

#undef APPEND_KEY_EVENT

    return send_rfb_message(sock, msg, pos) == pos ? 0 : -1;
}

 void osk_clear_one_shot_modifiers(void)
{
    if (osk_shift || osk_ctrl || osk_alt) {
        osk_shift = 0;
        osk_ctrl = 0;
        osk_alt = 0;
        osk_generation++;
    }
}

typedef struct {
    char c;
    unsigned char row[7];
} osk_glyph_t;

static const osk_glyph_t osk_font[] = {
    { ' ', { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { 'A', { 0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 } },
    { 'B', { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E } },
    { 'C', { 0x0F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0F } },
    { 'D', { 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E } },
    { 'E', { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F } },
    { 'F', { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10 } },
    { 'G', { 0x0F, 0x10, 0x10, 0x17, 0x11, 0x11, 0x0F } },
    { 'H', { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 } },
    { 'I', { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F } },
    { 'J', { 0x07, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0C } },
    { 'K', { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 } },
    { 'L', { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F } },
    { 'M', { 0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11 } },
    { 'N', { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 } },
    { 'O', { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E } },
    { 'P', { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 } },
    { 'Q', { 0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D } },
    { 'R', { 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11 } },
    { 'S', { 0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E } },
    { 'T', { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 } },
    { 'U', { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E } },
    { 'V', { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04 } },
    { 'W', { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A } },
    { 'X', { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11 } },
    { 'Y', { 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04 } },
    { 'Z', { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F } },
    { '0', { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E } },
    { '1', { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E } },
    { '2', { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F } },
    { '3', { 0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E } },
    { '4', { 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02 } },
    { '5', { 0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E } },
    { '6', { 0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E } },
    { '7', { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 } },
    { '8', { 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E } },
    { '9', { 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E } },
    { '-', { 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00 } },
    { '=', { 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00 } },
    { ';', { 0x00, 0x04, 0x00, 0x04, 0x04, 0x08, 0x00 } },
    { ',', { 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08 } },
    { '.', { 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06 } },
    { '/', { 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10 } },
    { '_', { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F } },
    { '+', { 0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00 } },
    { ':', { 0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00 } },
    { '<', { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02 } },
    { '>', { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08 } },
    { '?', { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04 } },
    { '!', { 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04 } },
    { '@', { 0x0E, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0E } },
    { '#', { 0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A } },
    { '$', { 0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04 } },
    { '%', { 0x19, 0x1A, 0x04, 0x08, 0x16, 0x06, 0x00 } },
    { '^', { 0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00 } },
    { '&', { 0x0C, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0D } },
    { '*', { 0x00, 0x15, 0x0E, 0x1F, 0x0E, 0x15, 0x00 } },
    { '(', { 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02 } },
    { ')', { 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08 } },
    { 'a', { 0x00, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F } },
    { 'b', { 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x1E } },
    { 'c', { 0x00, 0x00, 0x0F, 0x10, 0x10, 0x10, 0x0F } },
    { 'd', { 0x01, 0x01, 0x0D, 0x13, 0x11, 0x11, 0x0F } },
    { 'e', { 0x00, 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0F } },
    { 'f', { 0x06, 0x09, 0x08, 0x1C, 0x08, 0x08, 0x08 } },
    { 'g', { 0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x0E } },
    { 'h', { 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x11 } },
    { 'i', { 0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x0E } },
    { 'j', { 0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0C } },
    { 'k', { 0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12 } },
    { 'l', { 0x0C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E } },
    { 'm', { 0x00, 0x00, 0x1A, 0x15, 0x15, 0x15, 0x15 } },
    { 'n', { 0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11 } },
    { 'o', { 0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E } },
    { 'p', { 0x00, 0x00, 0x1E, 0x11, 0x1E, 0x10, 0x10 } },
    { 'q', { 0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x01 } },
    { 'r', { 0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10 } },
    { 's', { 0x00, 0x00, 0x0F, 0x10, 0x0E, 0x01, 0x1E } },
    { 't', { 0x08, 0x08, 0x1C, 0x08, 0x08, 0x09, 0x06 } },
    { 'u', { 0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0D } },
    { 'v', { 0x00, 0x00, 0x11, 0x11, 0x11, 0x0A, 0x04 } },
    { 'w', { 0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A } },
    { 'x', { 0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11 } },
    { 'y', { 0x00, 0x00, 0x11, 0x11, 0x0F, 0x01, 0x0E } },
    { 'z', { 0x00, 0x00, 0x1F, 0x02, 0x04, 0x08, 0x1F } },
    { 0x5B, { 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E } },
    { 0x5D, { 0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E } },
    { 0x5C, { 0x10, 0x08, 0x08, 0x04, 0x02, 0x02, 0x01 } },
    { 0x27, { 0x04, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00 } },
    { 0x22, { 0x0A, 0x0A, 0x04, 0x00, 0x00, 0x00, 0x00 } },
    { 0x7B, { 0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02 } },
    { 0x7D, { 0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08 } },
    { 0x7C, { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 } },
    { 0x60, { 0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00 } },
    { 0x7E, { 0x00, 0x00, 0x09, 0x16, 0x00, 0x00, 0x00 } }
};

#define OSK_FONT_COUNT (sizeof(osk_font) / sizeof(osk_font[0]))

enum {
    OSK_PAGE_ABC = 0,
    OSK_PAGE_FUNC = 1
};

/*
 * Page 1: normal US keyboard.
 * Shift supplies the symbols normally reached from this keyboard.
 */
static const char *osk_abc_rows[4] = {
    "1234567890-=",
    "QWERTYUIOP[]\\",
    "ASDFGHJKL;'",
    "ZXCVBNM,./`"
};

/*
 * Page 2: keys that cannot be reached from the compact ABC keyboard.
 *
 * Row 2/3 intentionally form a visual navigation cluster:
 *
 *        PRTSC        UP        PAUSE
 *        LEFT        DOWN       RIGHT
 */
static const char *osk_func_labels[4][12] = {
    {
        "F1", "F2", "F3", "F4",
        "F5", "F6", "F7", "F8",
        "F9", "F10", "F11", "F12"
    },
    {
        "HOME", "END", "PGUP", "PGDN", "INS", "CAPS"
    },
    {
        "PRTSC", "UP", "PAUSE"
    },
    {
        "LEFT", "DOWN", "RIGHT"
    }
};

 const unsigned int osk_func_keysyms[4][12] = {
    {
        XK_F1, XK_F2, XK_F3, XK_F4,
        XK_F5, XK_F6, XK_F7, XK_F8,
        XK_F9, XK_F10, XK_F11, XK_F12
    },
    {
        XK_HOME, XK_END, XK_PAGE_UP, XK_PAGE_DOWN,
        XK_INSERT, XK_CAPS_LOCK
    },
    {
        XK_PRINT, XK_UP, XK_PAUSE
    },
    {
        XK_LEFT, XK_DOWN, XK_RIGHT
    }
};

/*
 * Four content rows plus one permanent utility row.
 *
 * Bottom row on BOTH pages:
 *
 * ABC FUNC SHIFT CTRL ALT SPACE TAB BKSP DEL ENTER ESC
 */
static const int osk_page_row_lengths[2][OSK_ROWS] = {
    { 12, 13, 11, 11, 11 },  /* ABC  */
    { 12,  6,  3,  3, 11 }   /* FUNC */
};

static const char *osk_special_labels[11] = {
    "ABC", "FUNC",
    "SHIFT", "CTRL", "ALT",
    "SPACE", "TAB", "BKSP", "DEL", "ENTER", "ESC"
};

 int osk_row_length(int row)
{
    int page;

    if (row < 0 || row >= OSK_ROWS)
        return 0;

    page =
        (osk_page == OSK_PAGE_FUNC)
        ? OSK_PAGE_FUNC
        : OSK_PAGE_ABC;

    return osk_page_row_lengths[page][row];
}

unsigned short osk_color(
    unsigned int r5,
    unsigned int g5,
    unsigned int b5)
{
    return (unsigned short)(
        0x8000 |
        ((b5 & 0x1F) << 10) |
        ((g5 & 0x1F) << 5) |
        (r5 & 0x1F)
    );
}

void osk_fill_rect(
    int x,
    int y,
    int w,
    int h,
    unsigned short color)
{
    int yy;
    int xx;

    if (x < 0) {
        w += x;
        x = 0;
    }

    if (y < 0) {
        h += y;
        y = 0;
    }

    if (x + w > OSK_WIDTH)
        w = OSK_WIDTH - x;

    if (y + h > OSK_HEIGHT)
        h = OSK_HEIGHT - y;

    if (w <= 0 || h <= 0)
        return;

    for (yy = y; yy < y + h; yy++) {
        unsigned short *row =
            &osk_pixels[yy * OSK_WIDTH + x];

        for (xx = 0; xx < w; xx++)
            row[xx] = color;
    }
}

const unsigned char *osk_find_glyph(char c)
{
    unsigned int i;

    for (i = 0; i < OSK_FONT_COUNT; i++) {
        if (osk_font[i].c == c)
            return osk_font[i].row;
    }

    return NULL;
}

static void osk_draw_char(
    int x,
    int y,
    char c,
    int scale,
    unsigned short color)
{
    const unsigned char *glyph = osk_find_glyph(c);
    int gy;
    int gx;

    if (glyph == NULL)
        return;

    for (gy = 0; gy < 7; gy++) {
        for (gx = 0; gx < 5; gx++) {
            if (glyph[gy] & (1 << (4 - gx))) {
                osk_fill_rect(
                    x + gx * scale,
                    y + gy * scale,
                    scale,
                    scale,
                    color
                );
            }
        }
    }
}

void osk_draw_text_centered(
    int x,
    int y,
    int w,
    int h,
    const char *s,
    int scale,
    unsigned short color)
{
    int len = (int)strlen(s);
    int char_w = 6 * scale;
    int text_w = len * char_w - scale;
    int text_h = 7 * scale;
    int tx = x + (w - text_w) / 2;
    int ty = y + (h - text_h) / 2;
    int i;

    for (i = 0; i < len; i++) {
        osk_draw_char(tx + i * char_w, ty, s[i], scale, color);
    }
}

/*
 * Forward declaration: osk_display_char() uses this before its full
 * definition later in the file.
 */
 unsigned int osk_shifted_ascii(unsigned int c);

 char osk_display_char(int row, int col)
{
    unsigned int c;

    if (row < 0 || row >= 4)
        return ' ';

    c = (unsigned int)(unsigned char)
        osk_abc_rows[row][col];

    /*
     * Alphabet rows are stored uppercase for convenient layout.
     * With Shift off, send lowercase.
     */
    if (c >= 'A' && c <= 'Z')
        c = c - 'A' + 'a';

    if (osk_shift)
        c = osk_shifted_ascii(c);

    return (char)c;
}

 void osk_render_bitmap(void)
{
    const unsigned short panel = osk_color(3, 3, 4);
    const unsigned short key = osk_color(10, 10, 11);
    const unsigned short selected = osk_color(31, 24, 3);
    const unsigned short active = osk_color(6, 20, 8);
    const unsigned short border = osk_color(22, 22, 22);
    const unsigned short text_col = osk_color(31, 31, 31);
    const unsigned short selected_text = osk_color(1, 1, 1);

    int row;
    int col;
    int top = 4;
    int row_h = 33;

    osk_fill_rect(0, 0, OSK_WIDTH, OSK_HEIGHT, panel);

    for (row = 0; row < OSK_ROWS; row++) {
        int count = osk_row_length(row);
        int margin = 6;
        int gap = 3;
        int available =
            OSK_WIDTH - 2 * margin - (count - 1) * gap;
        int cell_w = available / count;
        int used_w =
            cell_w * count + gap * (count - 1);
        int row_x = (OSK_WIDTH - used_w) / 2;
        int y = top + row * row_h;
        int cell_h = row_h - 4;

        for (col = 0; col < count; col++) {
            int x = row_x + col * (cell_w + gap);

            int is_selected =
                (row == osk_row && col == osk_col);

            unsigned short bg =
                is_selected ? selected : key;

            unsigned short fg =
                is_selected ? selected_text : text_col;

            char label[8];
            int text_scale;

            if (row == 4) {
                strncpy(
                    label,
                    osk_special_labels[col],
                    sizeof(label) - 1
                );

                label[sizeof(label) - 1] = '\0';

                /*
                 * Persistent state indication:
                 *
                 * ABC / FUNC = active page
                 * Shift/Ctrl/Alt = latched modifier
                 *
                 * Cursor selection still takes visual priority.
                 */
                if (!is_selected &&
                    ((col == 0 &&
                      osk_page == OSK_PAGE_ABC) ||
                     (col == 1 &&
                      osk_page == OSK_PAGE_FUNC) ||
                     (col == 2 && osk_shift) ||
                     (col == 3 && osk_ctrl) ||
                     (col == 4 && osk_alt))) {
                    bg = active;
                }

            } else if (osk_page == OSK_PAGE_ABC) {
                label[0] =
                    osk_display_char(row, col);
                label[1] = '\0';

            } else {
                strncpy(
                    label,
                    osk_func_labels[row][col],
                    sizeof(label) - 1
                );

                label[sizeof(label) - 1] = '\0';
            }

            /*
             * Long labels use the compact font size.
             * F10/F11/F12 and the short utility keys still fit at 2x.
             */
            text_scale =
                (strlen(label) >= 5) ? 1 : 2;

            osk_fill_rect(
                x - 1, y - 1,
                cell_w + 2, cell_h + 2,
                border
            );

            osk_fill_rect(
                x, y,
                cell_w, cell_h,
                bg
            );

            osk_draw_text_centered(
                x, y,
                cell_w, cell_h,
                label,
                text_scale,
                fg
            );
        }
    }
}


int draw_recovery_status(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *osk_tex,
    const char *line1,
    const char *line2,
    int seconds_left)
{
    const u64 black =
        GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x00);

    const u64 tex_color =
        GS_SETREG_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x00);

    const unsigned short panel =
        osk_color(3, 3, 4);

    const unsigned short text_col =
        osk_color(31, 31, 31);

    const unsigned short warning_col =
        osk_color(31, 24, 3);

    char countdown[40];

    osk_fill_rect(
        0, 0,
        OSK_WIDTH, OSK_HEIGHT,
        panel
    );

    osk_draw_text_centered(
        0, 18,
        OSK_WIDTH, 32,
        line1,
        2,
        warning_col
    );

    osk_draw_text_centered(
        0, 65,
        OSK_WIDTH, 32,
        line2,
        2,
        text_col
    );

    if (seconds_left > 0) {
        snprintf(
            countdown,
            sizeof(countdown),
            "TIMEOUT IN %d SEC",
            seconds_left
        );

        osk_draw_text_centered(
            0, 116,
            OSK_WIDTH, 30,
            countdown,
            2,
            text_col
        );
    }

    /*
     * HIRES texture uploads are immediate. Do not overwrite a texture
     * while the previous multi-pass frame may still be consuming it.
     */
    if (ps2vnc_display_wait_texture_idle(gsGlobal) < 0)
        return -1;

    gsKit_texture_upload(
        gsGlobal,
        osk_tex
    );

    gsKit_clear(
        gsGlobal,
        black
    );

    gsKit_prim_sprite_texture(
        gsGlobal,
        osk_tex,
        ps2vnc_display_output_x(((float)OUTPUT_WIDTH - OSK_WIDTH) / 2.0f),
        ps2vnc_display_output_y(((float)OUTPUT_HEIGHT - OSK_HEIGHT) / 2.0f),
        0.0f, 0.0f,
        ps2vnc_display_output_x(((float)OUTPUT_WIDTH + OSK_WIDTH) / 2.0f),
        ps2vnc_display_output_y(((float)OUTPUT_HEIGHT + OSK_HEIGHT) / 2.0f),
        (float)OSK_WIDTH,
        (float)OSK_HEIGHT,
        2,
        tex_color
    );

    /*
     * TEST13E HIRES equivalent of the proven DRAW -> FLIP ordering.
     * HIRES consumes the completed draw queue over its scanline passes.
     */
    if (ps2vnc_display_finish_draw(gsGlobal) < 0)
        return -1;

    if (ps2vnc_display_flip(gsGlobal) < 0)
        return -1;

    return 0;
}


/*
 * Initial startup gets the same ten-second grace period for the
 * VNC server. The Ethernet link itself is already waited on by main().
 */
static int initial_rfb_connect_with_retry(
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer)
{
    const struct timespec retry_delay = {
        1,
        0
    };

    int attempt;

    for (attempt = 0;
         attempt < RECONNECT_MAX_ATTEMPTS;
         attempt++) {

        int sock;

        if (!ethGetNetIFLinkStatus()) {
            screenf(
                "Ethernet unavailable - retry %d/%d",
                attempt + 1,
                RECONNECT_MAX_ATTEMPTS
            );

            nanosleep(
                &retry_delay,
                NULL
            );

            continue;
        }

        reset_rfb_transport_state();

        screenf(
            "VNC connect attempt %d/%d",
            attempt + 1,
            RECONNECT_MAX_ATTEMPTS
        );

        sock = rfb_connect_and_handshake();

        if (sock >= 0) {
            int ok = 1;

            if (set_gs555_pixel_format(sock) < 0)
                ok = 0;

            if (ok && set_raw_encoding(sock) < 0)
                ok = 0;

            if (ok && request_framebuffer(sock, 0) < 0)
                ok = 0;

            if (ok &&
                receive_framebuffer_update(
                    sock,
                    rfb_framebuffer,
                    gs_framebuffer,
                    1,
                    0) < 0)
                ok = 0;

            /*
             * TEST13J-D1:
             * The ordinary complete Raw framebuffer is now safely in
             * memory. Probe desktop-resize capability separately.
             */
            if (ok &&
                probe_extended_desktop_size(
                    sock,
                    rfb_framebuffer,
                    gs_framebuffer) < 0)
                ok = 0;

            if (ok)
                return sock;

            close(sock);
            reset_rfb_transport_state();
        }

        nanosleep(
            &retry_delay,
            NULL
        );
    }

    return -1;
}


/*
 * Live-session recovery.
 *
 * old_sock belongs to the dead RFB stream and is never reused.
 * A successful return is a completely new RFB session that has already:
 *
 *   - completed protocol/security handshake
 *   - selected RGB565
 *   - selected Raw encoding
 *   - received a new complete framebuffer
 */
int recover_rfb_connection(
    int old_sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    GSTEXTURE *osk_tex)
{
    const struct timespec retry_delay = {
        1,
        0
    };

    const struct timespec final_delay = {
        0,
        500000000
    };

    int attempt;
    const unsigned int recovery_target_width = desktop_width;
    const unsigned int recovery_target_height = desktop_height;

    rfb_connected = 0;

    reset_rfb_transport_state();

    if (old_sock >= 0)
        close(old_sock);

    for (attempt = 0;
         attempt < RECONNECT_MAX_ATTEMPTS;
         attempt++) {

        int seconds_left =
            RECONNECT_TIMEOUT_SECONDS - attempt;

        int new_sock = -1;

        if (exit_requested ||
            controller_thread_error)
            return -1;

        if (!ethGetNetIFLinkStatus()) {

            if (draw_recovery_status(
                gsGlobal,
                osk_tex,
                "ETHERNET CONNECTION LOST",
                "ATTEMPTING TO REESTABLISH",
                seconds_left
            ) < 0)
                return -1;

            nanosleep(
                &retry_delay,
                NULL
            );

            continue;
        }

        if (draw_recovery_status(
            gsGlobal,
            osk_tex,
            "VNC CONNECTION LOST",
            "ATTEMPTING TO RECONNECT",
            seconds_left
        ) < 0)
            return -1;

        reset_rfb_transport_state();

        /*
         * A restarted TigerVNC returns at its bootstrap geometry.
         * Accept that ServerInit temporarily; the calibrated desktop
         * remains the recovery target saved above.
         */
        desktop_geometry_locked = 0;
        new_sock = rfb_connect_and_handshake();
        desktop_geometry_locked = 1;

        if (new_sock < 0) {
            desktop_width = recovery_target_width;
            desktop_height = recovery_target_height;
        }

        if (new_sock >= 0) {
            int ok = 1;

            if (set_gs555_pixel_format(
                    new_sock) < 0)
                ok = 0;

            if (ok &&
                set_raw_encoding(
                    new_sock) < 0)
                ok = 0;

            if (ok &&
                request_framebuffer(
                    new_sock, 0) < 0)
                ok = 0;

            if (ok &&
                receive_framebuffer_update(
                    new_sock,
                    rfb_framebuffer,
                    gs_framebuffer,
                    1,
                    1) < 0)
                ok = 0;

            /*
             * TEST13J-D1:
             * Re-establish resize capability on every fresh RFB session.
             */
            if (ok &&
                probe_extended_desktop_size(
                    new_sock,
                    rfb_framebuffer,
                    gs_framebuffer) < 0)
                ok = 0;

            if (ok &&
                (desktop_width != recovery_target_width ||
                 desktop_height != recovery_target_height)) {

                if (request_and_confirm_desktop_size(
                        new_sock,
                        rfb_framebuffer,
                        gs_framebuffer,
                        recovery_target_width,
                        recovery_target_height) < 0) {
                    ok = 0;
                }

                if (ok) {
                    desktop_width = recovery_target_width;
                    desktop_height = recovery_target_height;

                    if (set_raw_encoding(new_sock) < 0 ||
                        request_framebuffer(new_sock, 0) < 0 ||
                        receive_framebuffer_update(
                            new_sock,
                            rfb_framebuffer,
                            gs_framebuffer,
                            1,
                            1) < 0) {
                        ok = 0;
                    }
                }
            }

            if (ok) {
                /*
                 * Publish the authoritative replacement frame through the
                 * active presentation backend before live updates resume.
                 */
                if (ps2vnc_display_publish_full_desktop(
                        gsGlobal,
                        tex,
                        gs_framebuffer) < 0) {
                    close(new_sock);
                    reset_rfb_transport_state();
                    return -1;
                }

                if (set_live_encoding(new_sock) < 0)
                    ok = 0;

                if (ok) {
                    rfb_connected = 1;
                    return new_sock;
                }
            }

            close(new_sock);
            reset_rfb_transport_state();
            desktop_width = recovery_target_width;
            desktop_height = recovery_target_height;
        }

        nanosleep(
            &retry_delay,
            NULL
        );
    }

    if (draw_recovery_status(
        gsGlobal,
        osk_tex,
        "UNABLE TO RECONNECT",
        "RETURNING TO SYSTEM MENU",
        -1
    ) < 0)
        return -1;

    nanosleep(
        &final_delay,
        NULL
    );

    return -1;
}



/*
 * TEST15E3-E:
 * Execute the semantic manual Refresh action through the already-proven
 * live-session recovery transaction.
 */
/*
 * H3C rollback transport boundary.
 *
 * A blind-X or timeout may interrupt recv_exact() in the middle of an RFB
 * message. Never attempt the known-good display reconstruction on that byte
 * stream. Discard it and establish a fresh synchronized RFB session first.
 *
 * This intentionally stops BEFORE publishing through a GS backend. The
 * caller will reconstruct the saved known-good display after this succeeds.
 */
int recover_rfb_transport_for_display_rollback(
    int old_sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    const ps2vnc_video_mode_t *handshake_mode)
{
    const struct timespec retry_delay = {
        1,
        0
    };

    const ps2vnc_video_mode_t *saved_active_mode =
        active_video_mode;

    const int saved_geometry_locked =
        desktop_geometry_locked;

    int attempt;

    if (handshake_mode == NULL)
        return -1;

    /*
     * Controller-generated RFB events are discarded until the rollback
     * display and replacement stream are both completely ready.
     */
    rfb_connected = 0;

    reset_rfb_transport_state();

    if (old_sock >= 0)
        close(old_sock);

    for (attempt = 0;
         attempt < RECONNECT_MAX_ATTEMPTS;
         attempt++) {

        int new_sock = -1;
        int ok = 1;

        if (exit_requested ||
            controller_thread_error)
            break;

        if (!ethGetNetIFLinkStatus()) {
            nanosleep(&retry_delay, NULL);
            continue;
        }

        reset_rfb_transport_state();

        /*
         * The server may still be at candidate B geometry when rollback
         * reconnects.  The ordinary handshake wrapper derives its ServerInit
         * limit from active mode A, which creates a chicken-and-egg failure:
         * B is rejected before we can issue the resize that converges it to A.
         *
         * Accept any geometry that fits PS2VNC's allocated RFB backing store.
         * Geometry remains temporarily unlocked so ServerInit publishes the
         * server's ACTUAL starting dimensions.  The later display switch owns
         * convergence to the exact saved A profile.
         *
         * Do not alter active_display_backend here: the candidate GS object
         * still owns its backend resources until the later display switch
         * destroys it.
         */
        active_video_mode = handshake_mode;
        desktop_geometry_locked = 0;

        new_sock = rfb_connect_and_handshake_limited(
            VNC_WIDTH,
            VNC_HEIGHT);

        desktop_geometry_locked = saved_geometry_locked;
        active_video_mode = saved_active_mode;

        if (new_sock < 0) {
            nanosleep(&retry_delay, NULL);
            continue;
        }

        if (set_gs555_pixel_format(new_sock) < 0)
            ok = 0;

        if (ok &&
            set_raw_encoding(new_sock) < 0)
            ok = 0;

        if (ok &&
            request_framebuffer(new_sock, 0) < 0)
            ok = 0;

        if (ok &&
            receive_framebuffer_update(
                new_sock,
                rfb_framebuffer,
                gs_framebuffer,
                1,
                1) < 0)
            ok = 0;

        if (ok &&
            probe_extended_desktop_size(
                new_sock,
                rfb_framebuffer,
                gs_framebuffer) < 0)
            ok = 0;

        if (ok)
            return new_sock;

        close(new_sock);
        reset_rfb_transport_state();

        nanosleep(&retry_delay, NULL);
    }

    active_video_mode = saved_active_mode;
    desktop_geometry_locked = saved_geometry_locked;

    return -1;
}


int perform_manual_rfb_recovery(
    int old_sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    GSTEXTURE *osk_tex)
{
    int new_sock;

    /*
     * Claim the request BEFORE the replacement handshake. recv_exact()
     * watches this request only to escape the suspect old RFB stream.
     */
    runtime_action_request = RUNTIME_ACTION_NONE;

    system_menu_countdown_action = RUNTIME_ACTION_NONE;
    system_menu_countdown_seconds = 0;

    system_refresh_state = SYSTEM_REFRESH_REFRESHING;
    system_refresh_cooldown_started_tick = 0;

    system_menu_status = SYSTEM_MENU_STATUS_REFRESHING;
    system_menu_selection = SYSTEM_MENU_REFRESH;
    system_menu_visible = 1;

    /*
     * Preserve ordinary UI underlay across transport replacement.
     * System remains foreground by existing render/input priority.
     */
    osk_generation++;

    new_sock = recover_rfb_connection(
        old_sock,
        rfb_framebuffer,
        gs_framebuffer,
        gsGlobal,
        tex,
        osk_tex
    );

    if (new_sock < 0)
        return -1;

    system_refresh_state = SYSTEM_REFRESH_COOLDOWN;
    system_refresh_cooldown_started_tick =
        GetTimerSystemTime();

    system_menu_status = SYSTEM_MENU_STATUS_REFRESH_COMPLETE;
    system_menu_selection = SYSTEM_MENU_REFRESH;
    system_menu_visible = 1;
    osk_generation++;

    return new_sock;
}


/*
 * TEST13J-B: single-screen size + position calibration.
 *
 * width/height/offset select the exact usable safe desktop.
 *
 * The white border is visualization only and is drawn immediately OUTSIDE
 * that safe rectangle. The user expands/moves the safe desktop until the
 * external border disappears evenly into the television's overscan.
 *
 * The safe desktop itself always remains inside the physical raster.
 *
 * D-pad       = size
 * R1 + D-pad  = position
 * X            = accept
 * Triangle     = reset
 * Circle       = full 1280x720
 */
#define SCREEN_FIT_FINE_STEP            1
#define SCREEN_FIT_FAST_STEP            4
#define SCREEN_FIT_BORDER               4
#define SCREEN_FIT_CORNER_SIZE          56
#define SCREEN_FIT_REPEAT_DELAY_POLLS  18
#define SCREEN_FIT_REPEAT_POLLS         4

#define SCREEN_FIT_START_WIDTH   (OUTPUT_WIDTH)
#define SCREEN_FIT_START_HEIGHT  (OUTPUT_HEIGHT)

#define SCREEN_FIT_MIN_WIDTH     (OUTPUT_WIDTH  / 2)
#define SCREEN_FIT_MIN_HEIGHT    (OUTPUT_HEIGHT / 2)

#define SCREEN_FIT_ACTION_MOVE   0x10000u

int screen_fit_width = VNC_WIDTH;
int screen_fit_height = VNC_HEIGHT;
int screen_fit_offset_x = 0;
int screen_fit_offset_y = 0;
int screen_fit_full_bypass = 0;


/*
 * Validate the complete selected safe desktop rectangle.
 *
 * offset_x / offset_y are signed displacements from centered
 * placement, not absolute framebuffer coordinates.
 */

static int screen_fit_geometry_valid_for_mode(
    const ps2vnc_video_mode_t *mode,
    int width,
    int height,
    int offset_x,
    int offset_y)
{
    int output_width;
    int output_height;
    int remaining_x;
    int remaining_y;
    int centered_x;
    int centered_y;
    int max_positive_x;
    int max_positive_y;

    if (mode == NULL)
        return 0;

    output_width = (int)mode->raster_width;
    output_height = (int)mode->raster_height;

    /*
     * D17AL-F1:
     *
     * width/height are the exact selected safe desktop rectangle.
     * SCREEN_FIT_BORDER is visualization only and consumes no pixels.
     */
    if (width <= 0 || height <= 0)
        return 0;

    if (width > output_width ||
        height > output_height)
        return 0;

    remaining_x = output_width - width;
    remaining_y = output_height - height;

    centered_x = remaining_x / 2;
    centered_y = remaining_y / 2;

    max_positive_x = remaining_x - centered_x;
    max_positive_y = remaining_y - centered_y;

    if (offset_x < -centered_x ||
        offset_x > max_positive_x)
        return 0;

    if (offset_y < -centered_y ||
        offset_y > max_positive_y)
        return 0;

    return 1;
}



/*
 * Preserve the historical active-display acceptance gate.
 * Runtime mode management can validate a target mode explicitly through
 * screen_fit_geometry_valid_for_mode() without changing GS state.
 */
int screen_fit_geometry_valid(
    int width,
    int height,
    int offset_x,
    int offset_y)
{
    return screen_fit_geometry_valid_for_mode(
        active_video_mode,
        width,
        height,
        offset_x,
        offset_y
    );
}


/*
 * Single acceptance gate for screen-fit presets.
 *
 * Future config-file parsing must feed through here rather than
 * assigning screen_fit_* variables directly.
 */
int screen_fit_apply_preset(
    int width,
    int height,
    int offset_x,
    int offset_y,
    int full_bypass)
{
    if (full_bypass) {

        /*
         * FULL is one exact intentional preset.
         */
        if (width == OUTPUT_WIDTH &&
            height == OUTPUT_HEIGHT &&
            offset_x == 0 &&
            offset_y == 0) {

            screen_fit_width = OUTPUT_WIDTH;
            screen_fit_height = OUTPUT_HEIGHT;
            screen_fit_offset_x = 0;
            screen_fit_offset_y = 0;
            screen_fit_full_bypass = 1;

            return 1;
        }

    } else if (screen_fit_geometry_valid(
                   width,
                   height,
                   offset_x,
                   offset_y)) {

        screen_fit_width = width;
        screen_fit_height = height;
        screen_fit_offset_x = offset_x;
        screen_fit_offset_y = offset_y;
        screen_fit_full_bypass = 0;

        return 1;
    }

    /*
     * Reject the complete supplied preset.
     * Do not preserve any component from bad input.
     */
    screen_fit_width = SCREEN_FIT_START_WIDTH;
    screen_fit_height = SCREEN_FIT_START_HEIGHT;
    screen_fit_offset_x = 0;
    screen_fit_offset_y = 0;
    screen_fit_full_bypass = 0;

    return 0;
}



/*
 * TEST13J-D4-A1:
 * General PS2VNC configuration parsing foundation.
 *
 * The configuration format is intentionally human-readable and
 * extensible.  Blank lines and # comments are permitted.  Unknown
 * sections and unknown keys are ignored so a newer configuration file
 * can still be used safely by an older PS2VNC build.
 *
 * Only settings recognized by this build are interpreted.
 *
 * Current display settings:
 *
 *     [display]
 *     safe_width  = ...
 *     safe_height = ...
 *     safe_x      = ...
 *     safe_y      = ...
 *
 * safe_width / safe_height describe the ACTUAL usable VNC desktop.
 *
 * D17AL-F1: the white calibration border is visualization only.
 * Stored dimensions are never expanded or contracted to account for it.
 *
 * safe_x / safe_y use the existing signed, center-relative offset
 * convention.  Negative values are valid.
 *
 * Security rules:
 *
 *   - configuration is data, never code
 *   - no shell/expression/variable evaluation
 *   - decimal integers only for recognized numeric settings
 *   - integer overflow rejected
 *   - trailing junk rejected
 *   - embedded NUL rejected
 *   - duplicate recognized keys rejected
 *   - impossible geometry rejected
 *   - live screen_fit_* state is never changed by this parser
 *
 * Future code that applies the parsed result must still pass the
 * translated OUTER rectangle through screen_fit_apply_preset().
 */

#define PS2VNC_CONFIG_MAX_BYTES  16384
#define PS2VNC_CONFIG_MAX_LINE     512

enum {
    PS2VNC_CONFIG_NO_DISPLAY = 0,
    PS2VNC_CONFIG_OK = 1,
    PS2VNC_CONFIG_INVALID_TEXT = -1,
    PS2VNC_CONFIG_INVALID_GEOMETRY = -2
};






/*
 * Parse the global [display] startup_mode selector independently from
 * per-mode calibration. Return 1 when present and valid, 0 when absent,
 * and -1 when the recognized setting is malformed or unsupported.
 */
static int ps2vnc_config_parse_startup_mode(
    const char *text,
    unsigned int text_len,
    const ps2vnc_video_mode_t **out_mode)
{
    unsigned int pos = 0;
    unsigned int i;
    int in_display = 0;
    int display_seen = 0;
    int startup_seen = 0;

    if (text == NULL || out_mode == NULL)
        return -1;

    if (text_len > PS2VNC_CONFIG_MAX_BYTES)
        return -1;

    for (i = 0; i < text_len; i++) {
        if (text[i] == '\0')
            return -1;
    }

    while (pos < text_len) {
        char line[PS2VNC_CONFIG_MAX_LINE];
        char *work, *hash, *equals, *key, *value;
        unsigned int start = pos;
        unsigned int line_len;

        while (pos < text_len && text[pos] != '\n')
            pos++;

        line_len = pos - start;

        if (line_len > 0 &&
            text[start + line_len - 1] == '\r')
            line_len--;

        if (line_len >= sizeof(line))
            return -1;

        memcpy(line, text + start, line_len);
        line[line_len] = '\0';

        if (pos < text_len && text[pos] == '\n')
            pos++;

        hash = strchr(line, '#');
        if (hash != NULL)
            *hash = '\0';

        work = ps2vnc_config_trim_left(line);
        ps2vnc_config_trim_right(work);

        if (*work == '\0')
            continue;

        if (*work == '[') {
            size_t len = strlen(work);

            if (len < 3 || work[len - 1] != ']')
                return -1;

            work[len - 1] = '\0';
            work++;
            work = ps2vnc_config_trim_left(work);
            ps2vnc_config_trim_right(work);

            if (strcmp(work, "display") == 0) {
                if (display_seen)
                    return -1;
                display_seen = 1;
                in_display = 1;
            } else {
                in_display = 0;
            }

            continue;
        }

        if (!in_display)
            continue;

        equals = strchr(work, '=');
        if (equals == NULL)
            return -1;

        *equals = '\0';
        key = ps2vnc_config_trim_left(work);
        ps2vnc_config_trim_right(key);

        if (strcmp(key, "startup_mode") != 0)
            continue;

        if (startup_seen)
            return -1;

        value = ps2vnc_config_trim_left(equals + 1);
        ps2vnc_config_trim_right(value);

        if (*value == '\0' || strchr(value, '=') != NULL)
            return -1;

        *out_mode = ps2vnc_video_mode_by_name(value);

        if (*out_mode == NULL)
            return -1;

        startup_seen = 1;
    }

    return startup_seen ? 1 : 0;
}




static int ps2vnc_config_parse_display_section(
    const char *text,
    unsigned int text_len,
    const char *section_name,
    const ps2vnc_video_mode_t *mode,
    ps2vnc_display_config_t *out_config)
{
    enum {
        HAVE_SAFE_WIDTH  = 1u << 0,
        HAVE_SAFE_HEIGHT = 1u << 1,
        HAVE_SAFE_X      = 1u << 2,
        HAVE_SAFE_Y      = 1u << 3
    };

    const unsigned int all_display_keys =
        HAVE_SAFE_WIDTH |
        HAVE_SAFE_HEIGHT |
        HAVE_SAFE_X |
        HAVE_SAFE_Y;

    ps2vnc_display_config_t candidate;

    unsigned int seen = 0;
    unsigned int pos = 0;
    unsigned int i;

    int in_display_section = 0;
    int display_section_seen = 0;

    if (text == NULL ||
        section_name == NULL ||
        section_name[0] == '\0' ||
        mode == NULL ||
        out_config == NULL)
        return PS2VNC_CONFIG_INVALID_TEXT;

    if (text_len == 0)
        return PS2VNC_CONFIG_NO_DISPLAY;

    if (text_len > PS2VNC_CONFIG_MAX_BYTES)
        return PS2VNC_CONFIG_INVALID_TEXT;

    /*
     * text_len is authoritative.  Reject embedded NUL bytes so C string
     * operations cannot see a different document from the file loader.
     */
    for (i = 0; i < text_len; i++) {

        if (text[i] == '\0')
            return PS2VNC_CONFIG_INVALID_TEXT;
    }

    memset(
        &candidate,
        0,
        sizeof(candidate)
    );

    while (pos < text_len) {

        char line[PS2VNC_CONFIG_MAX_LINE];

        char *work;
        char *hash;
        char *equals;
        char *key;
        char *value;

        unsigned int start = pos;
        unsigned int line_len;

        while (pos < text_len &&
               text[pos] != '\n') {
            pos++;
        }

        line_len = pos - start;

        if (line_len > 0 &&
            text[start + line_len - 1] == '\r') {

            line_len--;
        }

        if (line_len >= sizeof(line))
            return PS2VNC_CONFIG_INVALID_TEXT;

        memcpy(
            line,
            text + start,
            line_len
        );

        line[line_len] = '\0';

        if (pos < text_len &&
            text[pos] == '\n') {
            pos++;
        }


        /*
         * Remove comments.
         *
         * There are deliberately no quoted strings in the current grammar,
         * so the first # always begins a comment.
         */
        hash = strchr(line, '#');

        if (hash != NULL)
            *hash = '\0';


        work = ps2vnc_config_trim_left(line);
        ps2vnc_config_trim_right(work);

        /*
         * Blank line or comment-only line.
         */
        if (*work == '\0')
            continue;


        /*
         * Section header.
         */
        if (*work == '[') {

            size_t len = strlen(work);

            if (len < 3 ||
                work[len - 1] != ']') {

                return PS2VNC_CONFIG_INVALID_TEXT;
            }

            work[len - 1] = '\0';
            work++;

            work = ps2vnc_config_trim_left(work);
            ps2vnc_config_trim_right(work);

            if (*work == '\0')
                return PS2VNC_CONFIG_INVALID_TEXT;

            if (strcmp(work, section_name) == 0) {

                /*
                 * A second [display] section would make ownership of
                 * duplicate settings ambiguous, so reject it.
                 */
                if (display_section_seen)
                    return PS2VNC_CONFIG_INVALID_TEXT;

                display_section_seen = 1;
                in_display_section = 1;

            } else {

                /*
                 * Unknown/future sections are intentionally ignored.
                 */
                in_display_section = 0;
            }

            continue;
        }


        /*
         * Unknown sections are opaque to this build.  Do not impose this
         * build's key/value grammar on future settings.
         */
        if (!in_display_section)
            continue;


        equals = strchr(work, '=');

        if (equals == NULL)
            return PS2VNC_CONFIG_INVALID_TEXT;

        *equals = '\0';

        key = ps2vnc_config_trim_left(work);
        ps2vnc_config_trim_right(key);

        if (*key == '\0')
            return PS2VNC_CONFIG_INVALID_TEXT;

        /*
         * Forward compatibility:
         *
         * Unknown future keys are opaque to this build.  Once we know the
         * key is not one of ours, do not interpret or validate its value.
         *
         * This permits future value syntaxes without making an older PS2VNC
         * reject the complete configuration.
         */
        if (strcmp(key, "safe_width") != 0 &&
            strcmp(key, "safe_height") != 0 &&
            strcmp(key, "safe_x") != 0 &&
            strcmp(key, "safe_y") != 0) {

            continue;
        }

        value = ps2vnc_config_trim_left(equals + 1);
        ps2vnc_config_trim_right(value);

        /*
         * Recognized current settings remain deliberately strict.
         */
        if (*value == '\0' ||
            strchr(value, '=') != NULL) {

            return PS2VNC_CONFIG_INVALID_TEXT;
        }


        if (strcmp(key, "safe_width") == 0) {

            if (seen & HAVE_SAFE_WIDTH)
                return PS2VNC_CONFIG_INVALID_TEXT;

            if (!ps2vnc_config_parse_int(
                    value,
                    0,
                    1,
                    &candidate.safe_width)) {

                return PS2VNC_CONFIG_INVALID_TEXT;
            }

            seen |= HAVE_SAFE_WIDTH;


        } else if (strcmp(key, "safe_height") == 0) {

            if (seen & HAVE_SAFE_HEIGHT)
                return PS2VNC_CONFIG_INVALID_TEXT;

            if (!ps2vnc_config_parse_int(
                    value,
                    0,
                    1,
                    &candidate.safe_height)) {

                return PS2VNC_CONFIG_INVALID_TEXT;
            }

            seen |= HAVE_SAFE_HEIGHT;


        } else if (strcmp(key, "safe_x") == 0) {

            if (seen & HAVE_SAFE_X)
                return PS2VNC_CONFIG_INVALID_TEXT;

            if (!ps2vnc_config_parse_int(
                    value,
                    1,
                    0,
                    &candidate.safe_x)) {

                return PS2VNC_CONFIG_INVALID_TEXT;
            }

            seen |= HAVE_SAFE_X;


        } else if (strcmp(key, "safe_y") == 0) {

            if (seen & HAVE_SAFE_Y)
                return PS2VNC_CONFIG_INVALID_TEXT;

            if (!ps2vnc_config_parse_int(
                    value,
                    1,
                    0,
                    &candidate.safe_y)) {

                return PS2VNC_CONFIG_INVALID_TEXT;
            }

            seen |= HAVE_SAFE_Y;


        } else {

            /*
             * Unknown keys inside a known section are intentionally
             * preserved for forward compatibility and ignored by this build.
             */
            continue;
        }
    }


    /*
     * No [display] section, or a documented template with no active
     * display values, simply means there is no stored preset yet.
     */
    if (!display_section_seen ||
        seen == 0) {

        return PS2VNC_CONFIG_NO_DISPLAY;
    }


    /*
     * Once any current display key is active, require the complete current
     * display preset.  This prevents accidentally combining a partly edited
     * file with unrelated compiled defaults.
     */
    if (seen != all_display_keys)
        return PS2VNC_CONFIG_INVALID_TEXT;


    /*
     * D17AL-F1:
     *
     * The human-readable configuration already stores the exact safe
     * rectangle.  No border expansion or contraction occurs here.
     */
    if (!screen_fit_geometry_valid_for_mode(
            mode,
            candidate.safe_width,
            candidate.safe_height,
            candidate.safe_x,
            candidate.safe_y)) {

        return PS2VNC_CONFIG_INVALID_GEOMETRY;
    }


    /*
     * Atomic publication.
     *
     * The caller sees nothing unless the entire recognized display preset
     * passed syntax, integer and geometry validation.
     */
    *out_config = candidate;

    return PS2VNC_CONFIG_OK;
}


/*
 * TEST15E2-B2:
 * Select the calibration belonging to an explicit physical video mode.
 *
 * New configuration prefers [display.<mode>].  The historical [display]
 * section remains a backward-compatible 1080i fallback while old configs
 * are migrated.  An invalid keyed preset is authoritative and is never
 * hidden by falling back to legacy data.
 */
static int ps2vnc_config_parse_display_for_mode(
    const char *text,
    unsigned int text_len,
    const ps2vnc_video_mode_t *mode,
    ps2vnc_display_config_t *out_config)
{
    char section_name[64];
    int name_len;
    int result;

    if (mode == NULL ||
        mode->name == NULL ||
        mode->name[0] == '\0')
        return PS2VNC_CONFIG_INVALID_TEXT;

    name_len = snprintf(
        section_name,
        sizeof(section_name),
        "display.%s",
        mode->name
    );

    if (name_len <= 0 ||
        name_len >= (int)sizeof(section_name))
        return PS2VNC_CONFIG_INVALID_TEXT;

    result = ps2vnc_config_parse_display_section(
        text,
        text_len,
        section_name,
        mode,
        out_config
    );

    if (result != PS2VNC_CONFIG_NO_DISPLAY)
        return result;

    /*
     * Legacy [display] became the 1080i compatibility slot when Test15
     * promoted 1080i to the production/default display mode.
     */
    if (strcmp(mode->name, "1080i") == 0) {
        return ps2vnc_config_parse_display_section(
            text,
            text_len,
            "display",
            mode,
            out_config
        );
    }

    return PS2VNC_CONFIG_NO_DISPLAY;
}



int ps2vnc_config_apply_display(
    const ps2vnc_display_config_t *config)
{
    if (config == NULL)
        return 0;

    return screen_fit_apply_preset(
        config->safe_width,
        config->safe_height,
        config->safe_x,
        config->safe_y,
        0
    );
}





/*
 * TEST13J-D4-A3:
 * Read-only configuration retrieval from the Pi.
 *
 * This is intentionally a separate TCP connection from VNC/RFB.
 *
 * Bootstrap:
 *
 *     PS2 -> PS2VNC_BOOTSTRAP_PI_IP:PS2VNC_CONFIG_PORT
 *
 * Request:
 *
 *     GET /ps2vnc.conf HTTP/1.0
 *
 * The Pi-side implementation is deliberately hidden behind HTTP.  The PS2
 * does not know or care where the file lives on the Pi filesystem.
 *
 * Safety:
 *
 *   - response header is bounded
 *   - response body is bounded by PS2VNC_CONFIG_MAX_BYTES
 *   - only HTTP status 200 is accepted
 *   - body remains untrusted and is passed through the validated parser
 *   - no received value mutates live display state except through
 *     ps2vnc_config_apply_display() -> screen_fit_apply_preset()
 */

int ps2vnc_display_config_loaded = 0;


static int ps2vnc_http_find_header_end(
    const unsigned char *data,
    unsigned int len)
{
    unsigned int i;

    if (data == NULL || len < 4)
        return -1;

    for (i = 0; i + 3 < len; i++) {

        if (data[i] == '\r' &&
            data[i + 1] == '\n' &&
            data[i + 2] == '\r' &&
            data[i + 3] == '\n') {

            return (int)(i + 4);
        }
    }

    return -1;
}


static int ps2vnc_http_status_is_200(
    const unsigned char *header,
    unsigned int header_len)
{
    /*
     * Accept HTTP/1.0 and HTTP/1.1.
     *
     * Expected prefix:
     *
     *     HTTP/1.x 200
     */
    if (header == NULL ||
        header_len < 12)
        return 0;

    if (memcmp(header, "HTTP/1.", 7) != 0)
        return 0;

    if (header[7] != '0' &&
        header[7] != '1')
        return 0;

    if (header[8] != ' ')
        return 0;

    if (header[9] != '2' ||
        header[10] != '0' ||
        header[11] != '0')
        return 0;

    return 1;
}



static const char *ps2vnc_backend_transaction_name(
    ps2vnc_backend_t backend)
{
    if (backend == PS2VNC_BACKEND_STANDARD)
        return "standard";

    if (backend == PS2VNC_BACKEND_HIRES)
        return "hires";

    return NULL;
}


static ps2vnc_backend_t ps2vnc_backend_from_transaction_name(
    const char *name)
{
    if (name == NULL)
        return PS2VNC_BACKEND_NONE;

    if (strcmp(name, "standard") == 0)
        return PS2VNC_BACKEND_STANDARD;

    if (strcmp(name, "hires") == 0)
        return PS2VNC_BACKEND_HIRES;

    return PS2VNC_BACKEND_NONE;
}



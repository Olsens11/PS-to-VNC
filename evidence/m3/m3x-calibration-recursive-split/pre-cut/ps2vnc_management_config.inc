
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
static int screen_fit_geometry_valid(
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


static int ps2vnc_display_profile_valid(
    const ps2vnc_display_profile_t *profile)
{
    unsigned int expected_logical_w;
    unsigned int expected_logical_h;

    if (profile == NULL ||
        profile->mode == NULL ||
        profile->mode->name == NULL)
        return 0;

    if (!ps2vnc_video_mode_supports_backend(
            profile->mode,
            profile->backend))
        return 0;

    if (ps2vnc_backend_transaction_name(
            profile->backend) == NULL)
        return 0;

    if (profile->logical_width == 0 ||
        profile->logical_height == 0 ||
        profile->logical_width > VNC_WIDTH ||
        profile->logical_height > VNC_HEIGHT ||
        profile->output_width == 0 ||
        profile->output_height == 0)
        return 0;

    if (profile->output_x > profile->mode->raster_width ||
        profile->output_y > profile->mode->raster_height)
        return 0;

    if (profile->output_width >
            profile->mode->raster_width - profile->output_x ||
        profile->output_height >
            profile->mode->raster_height - profile->output_y)
        return 0;

    if (profile->backend == PS2VNC_BACKEND_HIRES) {
        expected_logical_w = profile->output_width;
        expected_logical_h = profile->output_height;
    } else {
        expected_logical_w =
            profile->mode->standard_logical_width;
        expected_logical_h =
            profile->mode->standard_logical_height;
    }

    if (expected_logical_w == 0 ||
        expected_logical_h == 0 ||
        profile->logical_width != expected_logical_w ||
        profile->logical_height != expected_logical_h)
        return 0;

    return 1;
}


/*
 * H4B2A:
 * Capture the complete coherent runtime profile that is active now.
 */
int ps2vnc_display_profile_capture_active(
    ps2vnc_display_profile_t *profile)
{
    if (profile == NULL ||
        active_video_mode == NULL ||
        active_display_backend == PS2VNC_BACKEND_NONE)
        return -1;

    profile->mode = active_video_mode;
    profile->backend = active_display_backend;

    profile->logical_width = desktop_width;
    profile->logical_height = desktop_height;

    profile->output_x = desktop_output_x;
    profile->output_y = desktop_output_y;
    profile->output_width = desktop_output_width;
    profile->output_height = desktop_output_height;

    return ps2vnc_display_profile_valid(profile) ? 0 : -1;
}


/*
 * Resolve a complete candidate profile without changing any live state.
 *
 * HIRES logical geometry is the calibrated presentation rectangle.
 * Standard logical geometry is the mode's fixed gsKit framebuffer.
 */
int ps2vnc_display_profile_resolve_candidate(
    const ps2vnc_video_mode_t *mode,
    unsigned int out_x,
    unsigned int out_y,
    unsigned int out_w,
    unsigned int out_h,
    ps2vnc_display_profile_t *profile)
{
    ps2vnc_backend_t backend;

    if (mode == NULL || profile == NULL)
        return -1;

    backend = ps2vnc_video_mode_default_backend(mode);

    if (backend == PS2VNC_BACKEND_NONE)
        return -1;

    profile->mode = mode;
    profile->backend = backend;

    if (backend == PS2VNC_BACKEND_HIRES) {
        profile->logical_width = out_w;
        profile->logical_height = out_h;

    } else if (backend == PS2VNC_BACKEND_STANDARD) {
        profile->logical_width = mode->standard_logical_width;
        profile->logical_height = mode->standard_logical_height;

    } else {
        return -1;
    }

    profile->output_x = out_x;
    profile->output_y = out_y;
    profile->output_width = out_w;
    profile->output_height = out_h;

    return ps2vnc_display_profile_valid(profile) ? 0 : -1;
}


/*
 * Small management-channel POST primitive for display transactions.
 *
 * This connection is independent of RFB.  A transition is considered
 * accepted only after an HTTP 200 response header has been received.
 */
static int ps2vnc_management_post(
    const char *path,
    const char *body,
    unsigned int body_len)
{
    struct sockaddr_in server;
    char request[512];
    unsigned char response[PS2VNC_HTTP_MAX_HEADER];

    unsigned int response_len = 0;
    int request_len;
    int header_end = -1;
    int sock = -1;
    int result = -1;

    if (path == NULL ||
        path[0] != '/' ||
        body_len > 4096u ||
        (body_len > 0 && body == NULL))
        return -1;

    request_len = snprintf(
        request,
        sizeof(request),
        "POST %s HTTP/1.0\r\n"
        "Host: " PS2VNC_BOOTSTRAP_PI_IP "\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %u\r\n"
        "Connection: close\r\n"
        "\r\n",
        path,
        body_len);

    if (request_len <= 0 ||
        request_len >= (int)sizeof(request))
        return -1;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
        goto done;

    memset(&server, 0, sizeof(server));
    server.sin_len = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_port = htons(PS2VNC_CONFIG_PORT);
    server.sin_addr.s_addr =
        inet_addr(PS2VNC_BOOTSTRAP_PI_IP);

    if (connect(
            sock,
            (struct sockaddr *)&server,
            sizeof(server)) < 0)
        goto done;

    if (send_exact(
            sock,
            request,
            request_len) != request_len)
        goto done;

    if (body_len > 0 &&
        send_exact(
            sock,
            body,
            (int)body_len) != (int)body_len)
        goto done;

    while (response_len < sizeof(response)) {
        int n = recv(
            sock,
            response + response_len,
            (int)(sizeof(response) - response_len),
            0);

        if (n <= 0)
            break;

        response_len += (unsigned int)n;

        header_end = ps2vnc_http_find_header_end(
            response,
            response_len);

        if (header_end >= 0)
            break;
    }

    if (header_end < 0 ||
        (unsigned int)header_end >
            PS2VNC_HTTP_MAX_HEADER)
        goto done;

    if (!ps2vnc_http_status_is_200(
            response,
            (unsigned int)header_end))
        goto done;

    result = 0;

done:
    if (sock >= 0)
        close(sock);

    return result;
}


int ps2vnc_display_transaction_begin_to_pi(
    const ps2vnc_display_profile_t *previous,
    const ps2vnc_display_profile_t *candidate)
{
    const char *previous_backend;
    const char *candidate_backend;
    char body[1024];
    int body_len;

    if (!ps2vnc_display_profile_valid(previous) ||
        !ps2vnc_display_profile_valid(candidate))
        return -1;

    if (previous->mode == candidate->mode &&
        previous->backend == candidate->backend &&
        previous->logical_width == candidate->logical_width &&
        previous->logical_height == candidate->logical_height &&
        previous->output_x == candidate->output_x &&
        previous->output_y == candidate->output_y &&
        previous->output_width == candidate->output_width &&
        previous->output_height == candidate->output_height)
        return -1;

    previous_backend =
        ps2vnc_backend_transaction_name(previous->backend);
    candidate_backend =
        ps2vnc_backend_transaction_name(candidate->backend);

    if (previous_backend == NULL ||
        candidate_backend == NULL)
        return -1;

    body_len = snprintf(
        body,
        sizeof(body),
        "previous_mode=%s\n"
        "previous_backend=%s\n"
        "previous_logical_width=%u\n"
        "previous_logical_height=%u\n"
        "previous_output_x=%u\n"
        "previous_output_y=%u\n"
        "previous_output_width=%u\n"
        "previous_output_height=%u\n"
        "candidate_mode=%s\n"
        "candidate_backend=%s\n"
        "candidate_logical_width=%u\n"
        "candidate_logical_height=%u\n"
        "candidate_output_x=%u\n"
        "candidate_output_y=%u\n"
        "candidate_output_width=%u\n"
        "candidate_output_height=%u\n",
        previous->mode->name,
        previous_backend,
        previous->logical_width,
        previous->logical_height,
        previous->output_x,
        previous->output_y,
        previous->output_width,
        previous->output_height,
        candidate->mode->name,
        candidate_backend,
        candidate->logical_width,
        candidate->logical_height,
        candidate->output_x,
        candidate->output_y,
        candidate->output_width,
        candidate->output_height);

    if (body_len <= 0 ||
        body_len >= (int)sizeof(body))
        return -1;

    return ps2vnc_management_post(
        PS2VNC_DISPLAY_TX_BEGIN_PATH,
        body,
        (unsigned int)body_len);
}


int ps2vnc_display_transaction_commit_to_pi(void)
{
    return ps2vnc_management_post(
        PS2VNC_DISPLAY_TX_COMMIT_PATH,
        NULL,
        0);
}


int ps2vnc_display_transaction_restore_to_pi(void)
{
    return ps2vnc_management_post(
        PS2VNC_DISPLAY_TX_RESTORE_PATH,
        NULL,
        0);
}


int ps2vnc_display_transaction_restored_to_pi(void)
{
    return ps2vnc_management_post(
        PS2VNC_DISPLAY_TX_RESTORED_PATH,
        NULL,
        0);
}


int ps2vnc_display_transaction_ack_to_pi(void)
{
    return ps2vnc_management_post(
        PS2VNC_DISPLAY_TX_ACK_PATH,
        NULL,
        0);
}


static int ps2vnc_management_get(
    const char *path,
    unsigned char *out_body,
    unsigned int out_capacity,
    unsigned int *out_len)
{
    char request[256];
    int request_len;

    struct sockaddr_in server;

    unsigned char *response;

    unsigned int response_capacity;
    unsigned int response_len = 0;

    int header_end;
    unsigned int body_len;

    int sock = -1;
    int result = -1;

    if (path == NULL ||
        path[0] != '/' ||
        out_body == NULL ||
        out_len == NULL ||
        out_capacity == 0)
        return -1;

    *out_len = 0;

    request_len = snprintf(
        request,
        sizeof(request),
        "GET %s HTTP/1.0\r\n"
        "Host: " PS2VNC_BOOTSTRAP_PI_IP "\r\n"
        "Connection: close\r\n"
        "\r\n",
        path);

    if (request_len <= 0 ||
        request_len >= (int)sizeof(request))
        return -1;

    /*
     * Allocate enough temporary space for the caller's accepted body plus
     * the independently bounded HTTP header.
     */
    response_capacity =
        PS2VNC_HTTP_MAX_HEADER +
        out_capacity;

    response = (unsigned char *)malloc(
        response_capacity
    );

    if (response == NULL) {
        screenf("MGMT GET: allocation failed");
        return -1;
    }

    sock = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (sock < 0) {
        screenf("MGMT GET: socket failed");
        goto done;
    }

    memset(
        &server,
        0,
        sizeof(server)
    );

    server.sin_len = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_port = htons(
        PS2VNC_CONFIG_PORT
    );
    server.sin_addr.s_addr =
        inet_addr(PS2VNC_BOOTSTRAP_PI_IP);

    if (connect(
            sock,
            (struct sockaddr *)&server,
            sizeof(server)) < 0) {

        screenf("MGMT GET: service unavailable");
        goto done;
    }

    if (send_exact(
            sock,
            request,
            request_len) != request_len) {

        screenf("MGMT GET: request failed");
        goto done;
    }

    /*
     * HTTP/1.0 + Connection: close gives us an unambiguous response end.
     *
     * Never allow a server to make us write beyond the maximum accepted
     * header plus the caller-supplied body capacity.
     */
    for (;;) {
        int n;

        if (response_len >= response_capacity) {
            screenf("MGMT GET: response too large");
            goto done;
        }

        n = recv(
            sock,
            response + response_len,
            (int)(response_capacity - response_len),
            0
        );

        if (n < 0) {
            screenf("MGMT GET: receive failed");
            goto done;
        }

        if (n == 0)
            break;

        response_len += (unsigned int)n;

        /*
         * As soon as the header terminator exists, enforce its independent
         * upper bound.  A giant header must not consume the body allowance.
         */
        header_end =
            ps2vnc_http_find_header_end(
                response,
                response_len
            );

        if (header_end < 0 &&
            response_len >=
                PS2VNC_HTTP_MAX_HEADER) {

            screenf("MGMT GET: HTTP header too large");
            goto done;
        }

        if (header_end >= 0 &&
            (unsigned int)header_end >
                PS2VNC_HTTP_MAX_HEADER) {

            screenf("MGMT GET: HTTP header too large");
            goto done;
        }
    }

    header_end =
        ps2vnc_http_find_header_end(
            response,
            response_len
        );

    if (header_end < 0) {
        screenf("MGMT GET: bad HTTP response");
        goto done;
    }

    if (!ps2vnc_http_status_is_200(
            response,
            (unsigned int)header_end)) {

        screenf("MGMT GET: HTTP status not 200");
        goto done;
    }

    body_len =
        response_len -
        (unsigned int)header_end;

    if (body_len > out_capacity) {

        screenf("MGMT GET: body too large");
        goto done;
    }

    if (body_len > 0) {
        memcpy(
            out_body,
            response + header_end,
            body_len
        );
    }

    *out_len = body_len;
    result = 0;

done:
    if (sock >= 0)
        close(sock);

    free(response);

    return result;
}


/*
 * D17AL-F2 generic small management POST.
 *
 * Like configuration GET, this connection is owned only by the main
 * RFB/render thread.
 */

/*
 * F8J1D:
 *
 * Frequent /display-control/pending polling is best-effort management
 * traffic. A sick or half-responsive Pi management service must not be
 * able to stall the main RFB/render thread indefinitely.
 *
 * This helper owns one TOTAL monotonic deadline across:
 *
 *     nonblocking connect
 *     request send
 *     response receive
 *
 * Routine success and expected poll failure are intentionally silent.
 * Existing configuration and durable transaction management helpers are
 * unchanged.
 */
static int ps2vnc_socket_wait_bounded(
    int sock,
    int want_read,
    int want_write,
    u64 started_tick,
    unsigned int budget_us)
{
    fd_set read_fds;
    fd_set write_fds;

    fd_set *read_ptr = NULL;
    fd_set *write_ptr = NULL;

    struct timeval timeout;

    unsigned int elapsed_us;
    unsigned int remaining_us;

    int ready;

    elapsed_us =
        profile_ticks_us(
            GetTimerSystemTime() -
            started_tick);

    if (elapsed_us >= budget_us)
        return 0;

    remaining_us =
        budget_us - elapsed_us;

    timeout.tv_sec =
        (long)(
            remaining_us /
            1000000u);

    timeout.tv_usec =
        (long)(
            remaining_us %
            1000000u);

    if (want_read) {
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        read_ptr = &read_fds;
    }

    if (want_write) {
        FD_ZERO(&write_fds);
        FD_SET(sock, &write_fds);
        write_ptr = &write_fds;
    }

    ready =
        select(
            sock + 1,
            read_ptr,
            write_ptr,
            NULL,
            &timeout);

    if (ready <= 0)
        return ready;

    if (want_read &&
        !FD_ISSET(sock, &read_fds))
        return 0;

    if (want_write &&
        !FD_ISSET(sock, &write_fds))
        return 0;

    return 1;
}


int ps2vnc_management_get_bounded_quiet(
    const char *path,
    unsigned char *out_body,
    unsigned int out_capacity,
    unsigned int *out_len,
    unsigned int budget_us)
{
    char request[256];
    int request_len;

    struct sockaddr_in server;

    unsigned char *response = NULL;

    unsigned int response_capacity;
    unsigned int response_len = 0;
    unsigned int request_sent = 0;

    int header_end;
    unsigned int body_len;

    int sock = -1;
    int result = -1;

    int socket_flags;

    u64 started_tick;

    if (path == NULL ||
        path[0] != '/' ||
        out_body == NULL ||
        out_len == NULL ||
        out_capacity == 0 ||
        budget_us == 0)
        return -1;

    *out_len = 0;

    request_len =
        snprintf(
            request,
            sizeof(request),
            "GET %s HTTP/1.0\r\n"
            "Host: " PS2VNC_BOOTSTRAP_PI_IP "\r\n"
            "Connection: close\r\n"
            "\r\n",
            path);

    if (request_len <= 0 ||
        request_len >=
            (int)sizeof(request))
        return -1;

    response_capacity =
        PS2VNC_HTTP_MAX_HEADER +
        out_capacity;

    response =
        (unsigned char *)malloc(
            response_capacity);

    if (response == NULL)
        return -1;

    sock =
        socket(
            AF_INET,
            SOCK_STREAM,
            0);

    if (sock < 0)
        goto done;

    socket_flags =
        fcntl(
            sock,
            F_GETFL,
            0);

    if (socket_flags < 0)
        goto done;

    if (fcntl(
            sock,
            F_SETFL,
            socket_flags |
                O_NONBLOCK) < 0)
        goto done;

    memset(
        &server,
        0,
        sizeof(server));

    server.sin_len =
        sizeof(server);

    server.sin_family =
        AF_INET;

    server.sin_port =
        htons(
            PS2VNC_CONFIG_PORT);

    server.sin_addr.s_addr =
        inet_addr(
            PS2VNC_BOOTSTRAP_PI_IP);

    started_tick =
        GetTimerSystemTime();

    if (connect(
            sock,
            (struct sockaddr *)&server,
            sizeof(server)) < 0) {

        int connect_error = 0;
        socklen_t connect_error_len =
            sizeof(connect_error);

        if (errno != EINPROGRESS &&
            errno != EALREADY &&
            errno != EAGAIN &&
            errno != EWOULDBLOCK)
            goto done;

        if (ps2vnc_socket_wait_bounded(
                sock,
                0,
                1,
                started_tick,
                budget_us) <= 0)
            goto done;

        if (getsockopt(
                sock,
                SOL_SOCKET,
                SO_ERROR,
                &connect_error,
                &connect_error_len) < 0)
            goto done;

        if (connect_error != 0)
            goto done;
    }

    while (request_sent <
           (unsigned int)request_len) {

        int n =
            send(
                sock,
                request + request_sent,
                request_len -
                    (int)request_sent,
                MSG_DONTWAIT);

        if (n > 0) {
            request_sent +=
                (unsigned int)n;
            continue;
        }

        if (n == 0)
            goto done;

        if (errno != EAGAIN &&
            errno != EWOULDBLOCK)
            goto done;

        if (ps2vnc_socket_wait_bounded(
                sock,
                0,
                1,
                started_tick,
                budget_us) <= 0)
            goto done;
    }

    for (;;) {
        int n;

        if (response_len >=
            response_capacity)
            goto done;

        n =
            recv(
                sock,
                response +
                    response_len,
                (int)(
                    response_capacity -
                    response_len),
                MSG_DONTWAIT);

        if (n > 0) {
            response_len +=
                (unsigned int)n;

            header_end =
                ps2vnc_http_find_header_end(
                    response,
                    response_len);

            if (header_end < 0 &&
                response_len >=
                    PS2VNC_HTTP_MAX_HEADER)
                goto done;

            if (header_end >= 0 &&
                (unsigned int)header_end >
                    PS2VNC_HTTP_MAX_HEADER)
                goto done;

            continue;
        }

        if (n == 0)
            break;

        if (errno != EAGAIN &&
            errno != EWOULDBLOCK)
            goto done;

        if (ps2vnc_socket_wait_bounded(
                sock,
                1,
                0,
                started_tick,
                budget_us) <= 0)
            goto done;
    }

    header_end =
        ps2vnc_http_find_header_end(
            response,
            response_len);

    if (header_end < 0)
        goto done;

    if (!ps2vnc_http_status_is_200(
            response,
            (unsigned int)header_end))
        goto done;

    body_len =
        response_len -
        (unsigned int)header_end;

    if (body_len > out_capacity)
        goto done;

    if (body_len > 0) {
        memcpy(
            out_body,
            response + header_end,
            body_len);
    }

    *out_len = body_len;
    result = 0;

done:
    if (sock >= 0)
        close(sock);

    free(response);

    return result;
}


int ps2vnc_management_post_text(
    const char *path,
    const char *body)
{
    struct sockaddr_in server;

    char request[512];

    unsigned char response[
        PS2VNC_HTTP_MAX_HEADER
    ];

    unsigned int response_len = 0;

    int body_len;
    int request_len;
    int header_end = -1;
    int sock = -1;
    int result = -1;

    if (path == NULL ||
        path[0] != '/' ||
        body == NULL)
        return -1;

    body_len = (int)strlen(body);

    if (body_len <= 0 ||
        body_len > 2048)
        return -1;

    request_len = snprintf(
        request,
        sizeof(request),
        "POST %s HTTP/1.0\r\n"
        "Host: " PS2VNC_BOOTSTRAP_PI_IP "\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        path,
        body_len
    );

    if (request_len <= 0 ||
        request_len >= (int)sizeof(request))
        return -1;

    sock = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (sock < 0)
        goto done;

    memset(
        &server,
        0,
        sizeof(server)
    );

    server.sin_len = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_port = htons(
        PS2VNC_CONFIG_PORT
    );

    server.sin_addr.s_addr =
        inet_addr(PS2VNC_BOOTSTRAP_PI_IP);

    if (connect(
            sock,
            (struct sockaddr *)&server,
            sizeof(server)) < 0)
        goto done;

    if (send_exact(
            sock,
            request,
            request_len) != request_len)
        goto done;

    if (send_exact(
            sock,
            body,
            body_len) != body_len)
        goto done;

    while (response_len <
           sizeof(response)) {

        int n = recv(
            sock,
            response + response_len,
            (int)(
                sizeof(response) -
                response_len),
            0
        );

        if (n <= 0)
            break;

        response_len +=
            (unsigned int)n;

        header_end =
            ps2vnc_http_find_header_end(
                response,
                response_len
            );

        if (header_end >= 0)
            break;
    }

    if (header_end < 0 ||
        (unsigned int)header_end >
            PS2VNC_HTTP_MAX_HEADER)
        goto done;

    if (!ps2vnc_http_status_is_200(
            response,
            (unsigned int)header_end))
        goto done;

    result = 0;

done:
    if (sock >= 0)
        close(sock);

    return result;
}




/*
 * Parse only menu policy from the human-readable config.
 *
 * Missing lock/hide keys are false. Unknown/future keys remain opaque.
 */
static int ps2vnc_config_parse_display_policy(
    const char *text,
    unsigned int text_len,
    unsigned char *out_locked,
    int *out_hide_locked)
{
    unsigned int pos = 0;
    unsigned int i;

    unsigned char locked_seen[
        VIDEO_MODE_COUNT
    ];

    int hide_seen = 0;
    int current_mode = -2;

    if (text == NULL ||
        out_locked == NULL ||
        out_hide_locked == NULL)
        return -1;

    if (text_len > PS2VNC_CONFIG_MAX_BYTES)
        return -1;

    for (i = 0; i < text_len; i++) {
        if (text[i] == '\0')
            return -1;
    }

    memset(
        out_locked,
        0,
        VIDEO_MODE_COUNT
    );

    memset(
        locked_seen,
        0,
        sizeof(locked_seen)
    );

    *out_hide_locked = 0;

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
               text[pos] != '\n')
            pos++;

        line_len = pos - start;

        if (line_len > 0 &&
            text[start + line_len - 1] == '\r')
            line_len--;

        if (line_len >= sizeof(line))
            return -1;

        memcpy(
            line,
            text + start,
            line_len
        );

        line[line_len] = '\0';

        if (pos < text_len &&
            text[pos] == '\n')
            pos++;

        hash = strchr(line, '#');
        if (hash != NULL)
            *hash = '\0';

        work =
            ps2vnc_config_trim_left(line);

        ps2vnc_config_trim_right(work);

        if (*work == '\0')
            continue;

        if (*work == '[') {
            size_t len = strlen(work);

            current_mode = -2;

            if (len < 3 ||
                work[len - 1] != ']')
                continue;

            work[len - 1] = '\0';
            work++;

            work =
                ps2vnc_config_trim_left(work);

            ps2vnc_config_trim_right(work);

            if (strcmp(work, "display") == 0) {

                current_mode = -1;

            } else if (
                strncmp(
                    work,
                    "display.",
                    8) == 0) {

                const ps2vnc_video_mode_t *mode =
                    ps2vnc_video_mode_by_name(
                        work + 8);

                if (mode != NULL)
                    current_mode =
                        display_menu_selection_for_mode(
                            mode);
            }

            continue;
        }

        if (current_mode < -1)
            continue;

        equals = strchr(work, '=');

        if (equals == NULL)
            continue;

        *equals = '\0';

        key =
            ps2vnc_config_trim_left(work);

        ps2vnc_config_trim_right(key);

        value =
            ps2vnc_config_trim_left(
                equals + 1);

        ps2vnc_config_trim_right(value);

        if (current_mode == -1 &&
            strcmp(
                key,
                "hide_locked_modes") == 0) {

            int parsed;

            if (hide_seen ||
                !ps2vnc_config_parse_bool(
                    value,
                    &parsed))
                return -1;

            hide_seen = 1;
            *out_hide_locked = parsed;

        } else if (
            current_mode >= 0 &&
            strcmp(key, "locked") == 0) {

            int parsed;

            if (locked_seen[current_mode] ||
                !ps2vnc_config_parse_bool(
                    value,
                    &parsed))
                return -1;

            locked_seen[current_mode] = 1;
            out_locked[current_mode] =
                parsed ? 1 : 0;
        }
    }

    return 0;
}


int ps2vnc_display_policy_refresh_from_pi(void)
{
    unsigned char *text;

    unsigned char new_locked[
        VIDEO_MODE_COUNT
    ];

    unsigned int len = 0;
    unsigned int i;
    int new_hide = 0;
    int result = -1;

    text = (unsigned char *)malloc(
        PS2VNC_CONFIG_MAX_BYTES + 1
    );

    if (text == NULL)
        return -1;

    if (ps2vnc_management_get(
            PS2VNC_CONFIG_PATH,
            text,
            PS2VNC_CONFIG_MAX_BYTES,
            &len) < 0)
        goto done;

    if (len > PS2VNC_CONFIG_MAX_BYTES)
        goto done;

    text[len] = '\0';

    if (ps2vnc_config_parse_display_policy(
            (const char *)text,
            len,
            new_locked,
            &new_hide) < 0)
        goto done;

    /*
     * Publish the complete cache only after the entire document parses.
     */
    for (i = 0;
         i < VIDEO_MODE_COUNT;
         i++) {

        display_mode_locked[i] =
            new_locked[i];
    }

    display_hide_locked_modes =
        new_hide ? 1 : 0;

    display_policy_loaded = 1;
    result = 0;

done:
    free(text);
    return result;
}


int ps2vnc_display_policy_lock_to_pi(
    int selection,
    int locked)
{
    const ps2vnc_video_mode_t *mode =
        display_menu_mode_at_selection(
            selection);

    char body[160];
    int body_len;

    if (mode == NULL ||
        mode == active_video_mode)
        return -1;

    body_len = snprintf(
        body,
        sizeof(body),
        "mode = %s\n"
        "locked = %s\n",
        mode->name,
        locked ? "true" : "false"
    );

    if (body_len <= 0 ||
        body_len >= (int)sizeof(body))
        return -1;

    return ps2vnc_management_post_text(
        PS2VNC_DISPLAY_LOCK_PATH,
        body
    );
}


int ps2vnc_display_policy_hide_to_pi(
    int hide_locked)
{
    char body[96];
    int body_len;

    body_len = snprintf(
        body,
        sizeof(body),
        "hide_locked_modes = %s\n",
        hide_locked ? "true" : "false"
    );

    if (body_len <= 0 ||
        body_len >= (int)sizeof(body))
        return -1;

    return ps2vnc_management_post_text(
        PS2VNC_DISPLAY_UI_PATH,
        body
    );
}



/*
 * H4C3B2A:
 * Consume exactly one newline-terminated key=value field.
 *
 * The H4A transaction format is versioned and deliberately strict.
 * Unknown, duplicated, reordered-by-omission, malformed, or unterminated
 * fields therefore fail closed rather than being guessed at.
 */
static int ps2vnc_display_tx_take_line(
    char **cursor,
    char *end,
    const char *key,
    char **out_value)
{
    char *line;
    char *newline;
    unsigned int key_len;

    if (cursor == NULL ||
        *cursor == NULL ||
        end == NULL ||
        key == NULL ||
        out_value == NULL ||
        *cursor >= end)
        return 0;

    line = *cursor;

    newline = (char *)memchr(
        line,
        '\n',
        (size_t)(end - line));

    if (newline == NULL)
        return 0;

    *newline = '\0';

    key_len = (unsigned int)strlen(key);

    if ((unsigned int)(newline - line) <= key_len ||
        memcmp(line, key, key_len) != 0 ||
        line[key_len] != '=')
        return 0;

    *out_value = line + key_len + 1;

    if ((*out_value)[0] == '\0' ||
        strchr(*out_value, '=') != NULL)
        return 0;

    *cursor = newline + 1;
    return 1;
}


static int ps2vnc_display_tx_parse_uint(
    const char *text,
    int require_positive,
    unsigned int *out_value)
{
    int value;

    if (out_value == NULL ||
        !ps2vnc_config_parse_int(
            text,
            0,
            require_positive,
            &value))
        return 0;

    *out_value = (unsigned int)value;
    return 1;
}


static int ps2vnc_display_tx_take_profile_value(
    char **cursor,
    char *end,
    const char *prefix,
    const char *field,
    char **out_value)
{
    char key[64];
    int n;

    n = snprintf(
        key,
        sizeof(key),
        "%s_%s",
        prefix,
        field);

    if (n <= 0 || n >= (int)sizeof(key))
        return 0;

    return ps2vnc_display_tx_take_line(
        cursor,
        end,
        key,
        out_value);
}


static int ps2vnc_display_tx_parse_profile(
    char **cursor,
    char *end,
    const char *prefix,
    ps2vnc_display_profile_t *profile)
{
    char *value;

    if (cursor == NULL ||
        prefix == NULL ||
        profile == NULL)
        return -1;

    memset(profile, 0, sizeof(*profile));

    if (!ps2vnc_display_tx_take_profile_value(
            cursor, end, prefix, "mode", &value))
        return -1;

    profile->mode = ps2vnc_video_mode_by_name(value);

    if (profile->mode == NULL)
        return -1;

    if (!ps2vnc_display_tx_take_profile_value(
            cursor, end, prefix, "backend", &value))
        return -1;

    profile->backend =
        ps2vnc_backend_from_transaction_name(value);

    if (profile->backend == PS2VNC_BACKEND_NONE)
        return -1;

#define H4C_TX_UINT_FIELD(FIELD, MEMBER, POSITIVE) do {             \
        if (!ps2vnc_display_tx_take_profile_value(                  \
                cursor, end, prefix, FIELD, &value) ||              \
            !ps2vnc_display_tx_parse_uint(                          \
                value, POSITIVE, &profile->MEMBER))                 \
            return -1;                                              \
    } while (0)

    H4C_TX_UINT_FIELD(
        "logical_width", logical_width, 1);
    H4C_TX_UINT_FIELD(
        "logical_height", logical_height, 1);
    H4C_TX_UINT_FIELD(
        "output_x", output_x, 0);
    H4C_TX_UINT_FIELD(
        "output_y", output_y, 0);
    H4C_TX_UINT_FIELD(
        "output_width", output_width, 1);
    H4C_TX_UINT_FIELD(
        "output_height", output_height, 1);

#undef H4C_TX_UINT_FIELD

    return ps2vnc_display_profile_valid(profile) ? 0 : -1;
}


int ps2vnc_display_tx_profiles_equal(
    const ps2vnc_display_profile_t *a,
    const ps2vnc_display_profile_t *b)
{
    return a != NULL &&
           b != NULL &&
           a->mode == b->mode &&
           a->backend == b->backend &&
           a->logical_width == b->logical_width &&
           a->logical_height == b->logical_height &&
           a->output_x == b->output_x &&
           a->output_y == b->output_y &&
           a->output_width == b->output_width &&
           a->output_height == b->output_height;
}

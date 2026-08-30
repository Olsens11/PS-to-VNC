

static int ps2vnc_display_tx_parse(
    char *text,
    unsigned int text_len,
    ps2vnc_display_transaction_t *transaction)
{
    char *cursor;
    char *end;
    char *value;

    if (text == NULL ||
        text_len == 0 ||
        transaction == NULL)
        return -1;

    memset(transaction, 0, sizeof(*transaction));

    /*
     * H4A emits this exact single-line body when no transaction exists.
     */
    if (text_len == sizeof("state=none\n") - 1 &&
        memcmp(
            text,
            "state=none\n",
            sizeof("state=none\n") - 1) == 0) {

        transaction->state = PS2VNC_DISPLAY_TX_NONE;
        return 0;
    }

    cursor = text;
    end = text + text_len;

    if (!ps2vnc_display_tx_take_line(
            &cursor, end, "version", &value) ||
        strcmp(value, "1") != 0)
        return -1;

    if (!ps2vnc_display_tx_take_line(
            &cursor, end, "state", &value))
        return -1;

    if (strcmp(value, "provisional") == 0)
        transaction->state = PS2VNC_DISPLAY_TX_PROVISIONAL;
    else if (strcmp(value, "committing") == 0)
        transaction->state = PS2VNC_DISPLAY_TX_COMMITTING;
    else if (strcmp(value, "restoring") == 0)
        transaction->state = PS2VNC_DISPLAY_TX_RESTORING;
    else if (strcmp(value, "restored") == 0)
        transaction->state = PS2VNC_DISPLAY_TX_RESTORED;
    else
        return -1;

    if (ps2vnc_display_tx_parse_profile(
            &cursor,
            end,
            "previous",
            &transaction->previous) < 0)
        return -1;

    if (ps2vnc_display_tx_parse_profile(
            &cursor,
            end,
            "candidate",
            &transaction->candidate) < 0)
        return -1;

    /*
     * The serializer terminates the final field with exactly one newline.
     * Anything remaining is an unknown/duplicate field and is rejected.
     */
    if (cursor != end)
        return -1;

    if (ps2vnc_display_tx_profiles_equal(
            &transaction->previous,
            &transaction->candidate))
        return -1;

    return 0;
}


static int ps2vnc_display_transaction_fetch_from_pi(
    ps2vnc_display_transaction_t *transaction)
{
    unsigned char *body;
    unsigned int body_len = 0;
    int result = -1;

    if (transaction == NULL)
        return -1;

    body = (unsigned char *)malloc(
        PS2VNC_DISPLAY_TX_MAX_BYTES + 1);

    if (body == NULL)
        return -1;

    if (ps2vnc_management_get(
            PS2VNC_DISPLAY_TX_PATH,
            body,
            PS2VNC_DISPLAY_TX_MAX_BYTES,
            &body_len) < 0)
        goto done;

    if (body_len == 0 ||
        body_len > PS2VNC_DISPLAY_TX_MAX_BYTES)
        goto done;

    body[body_len] = '\0';

    if (ps2vnc_display_tx_parse(
            (char *)body,
            body_len,
            transaction) < 0)
        goto done;

    result = 0;

done:
    free(body);
    return result;
}


static const char *ps2vnc_display_tx_state_name(
    ps2vnc_display_tx_state_t state)
{
    switch (state) {
        case PS2VNC_DISPLAY_TX_NONE:
            return "none";

        case PS2VNC_DISPLAY_TX_PROVISIONAL:
            return "provisional";

        case PS2VNC_DISPLAY_TX_COMMITTING:
            return "committing";

        case PS2VNC_DISPLAY_TX_RESTORING:
            return "restoring";

        case PS2VNC_DISPLAY_TX_RESTORED:
            return "restored";

        default:
            return "invalid";
    }
}


/*
 * H4C3B2B:
 * Establish durable display authority before opening any RFB connection.
 *
 * This intermediate checkpoint deliberately permits only state=none.
 * All durable transaction states are recognized and preserved, but remain
 * fail-closed until H4C wires their explicit startup reconciliation paths.
 */
static int ps2vnc_display_transaction_select_startup_authority(void)
{
    const ps2vnc_display_profile_t *source = NULL;

    startup_authoritative_display_profile_loaded = 0;

    startup_display_commit_finish_required = 0;
    startup_display_restore_required = 0;
    startup_display_restored_mark_required = 0;
    startup_display_restore_ack_required = 0;

    memset(
        &startup_authoritative_display_profile,
        0,
        sizeof(startup_authoritative_display_profile));

    if (!startup_display_transaction_loaded)
        return -1;

    switch (startup_display_transaction.state) {

        case PS2VNC_DISPLAY_TX_NONE:

            /*
             * No durable transaction means confirmed ps2vnc.conf wins.
             *
             * A machine without a stored preset retains the historical
             * interactive-calibration startup path, so absence of a
             * confirmed profile is not itself an error here.
             */
            if (!startup_confirmed_display_profile_loaded)
                return 0;

            source = &startup_confirmed_display_profile;
            break;

        case PS2VNC_DISPLAY_TX_COMMITTING:

            /*
             * KEEP was durably authorized. Candidate B wins even if the
             * process died before the persistence sequence completely
             * settled.
             *
             * During that durable commit window the persistent config may
             * still be complete previous A or may already be complete
             * candidate B. A third/mixed profile is never valid.
             */
            if (startup_confirmed_display_profile_loaded &&
                !ps2vnc_display_tx_profiles_equal(
                    &startup_confirmed_display_profile,
                    &startup_display_transaction.previous) &&
                !ps2vnc_display_tx_profiles_equal(
                    &startup_confirmed_display_profile,
                    &startup_display_transaction.candidate)) {

                screenf("DISPLAY TX: mixed committing config");
                return -1;
            }

            source = &startup_display_transaction.candidate;
            startup_display_commit_finish_required = 1;
            break;

        case PS2VNC_DISPLAY_TX_PROVISIONAL:

            /*
             * KEEP was never authorized. Previous A wins and this fresh
             * process must first durably enter RESTORING before repairing A.
             *
             * Persistent confirmed state must still describe complete A.
             * Any mismatch is a forbidden hybrid/corrupt state.
             */
            if (!startup_confirmed_display_profile_loaded ||
                !ps2vnc_display_tx_profiles_equal(
                    &startup_confirmed_display_profile,
                    &startup_display_transaction.previous)) {

                screenf("DISPLAY TX: previous/config mismatch");
                return -1;
            }

            source = &startup_display_transaction.previous;
            startup_display_restore_required = 1;
            startup_display_restored_mark_required = 1;
            startup_display_restore_ack_required = 1;
            break;

        case PS2VNC_DISPLAY_TX_RESTORING:

            if (!startup_confirmed_display_profile_loaded ||
                !ps2vnc_display_tx_profiles_equal(
                    &startup_confirmed_display_profile,
                    &startup_display_transaction.previous)) {

                screenf("DISPLAY TX: previous/config mismatch");
                return -1;
            }

            source = &startup_display_transaction.previous;
            startup_display_restored_mark_required = 1;
            startup_display_restore_ack_required = 1;
            break;

        case PS2VNC_DISPLAY_TX_RESTORED:

            if (!startup_confirmed_display_profile_loaded ||
                !ps2vnc_display_tx_profiles_equal(
                    &startup_confirmed_display_profile,
                    &startup_display_transaction.previous)) {

                screenf("DISPLAY TX: previous/config mismatch");
                return -1;
            }

            source = &startup_display_transaction.previous;
            startup_display_restore_ack_required = 1;
            break;

        default:
            return -1;
    }

    if (source == NULL ||
        !ps2vnc_display_profile_valid(source))
        return -1;

    startup_authoritative_display_profile = *source;
    startup_authoritative_display_profile_loaded = 1;

    return 0;
}


static int ps2vnc_display_transaction_startup_preflight(void)
{
    startup_display_transaction_loaded = 0;

    memset(
        &startup_display_transaction,
        0,
        sizeof(startup_display_transaction));

    if (ps2vnc_display_transaction_fetch_from_pi(
            &startup_display_transaction) < 0) {

        screenf("DISPLAY TX: startup read failed");
        return -1;
    }

    startup_display_transaction_loaded = 1;

    screenf(
        "DISPLAY TX: startup state %s",
        ps2vnc_display_tx_state_name(
            startup_display_transaction.state));

    if (ps2vnc_display_transaction_select_startup_authority() < 0) {
        screenf("DISPLAY TX: authority invalid");
        return -1;
    }

    if (startup_authoritative_display_profile_loaded) {
        screenf(
            "DISPLAY TX: authority %s %ux%u",
            startup_authoritative_display_profile.mode->name,
            startup_authoritative_display_profile.logical_width,
            startup_authoritative_display_profile.logical_height);
    } else {
        screenf("DISPLAY TX: no confirmed geometry");
    }

    if (startup_display_transaction.state !=
            PS2VNC_DISPLAY_TX_NONE) {

        screenf(
            "DISPLAY TX: startup recovery %s",
            ps2vnc_display_tx_state_name(
                startup_display_transaction.state));
    }

    return 0;
}


/*
 * H4C3C3C:
 * Prepare durable startup recovery before any RFB/display reconstruction.
 *
 * A provisional candidate was never accepted.  On fresh startup this is
 * semantically identical to GO BACK / timeout, so publish RESTORING before
 * beginning to reconstruct complete previous A.
 *
 * Other durable states already encode the correct authority decision.
 */
static int ps2vnc_startup_prepare_display_transaction(void)
{
    if (!startup_display_transaction_loaded)
        return -1;

    if (!startup_display_restore_required)
        return 0;

    if (startup_display_transaction.state !=
            PS2VNC_DISPLAY_TX_PROVISIONAL) {

        screenf("DISPLAY TX: restore preparation mismatch");
        return -1;
    }

    if (ps2vnc_display_transaction_restore_to_pi() < 0) {
        screenf("DISPLAY TX: startup RESTORE failed");
        return -1;
    }

    startup_display_transaction.state =
        PS2VNC_DISPLAY_TX_RESTORING;

    startup_display_restore_required = 0;

    screenf("DISPLAY TX: startup RESTORING");

    return 0;
}



/*
 * H4C3C2B:
 * Establish an RFB session coherent with the already-selected complete
 * authoritative startup profile.
 *
 * Durable startup state has already selected exactly one complete authority
 * and provisional has already been durably advanced to RESTORING.
 *
 * The server may have been stranded at another valid PS2VNC geometry by an
 * interrupted earlier run. The special handshake therefore accepts any
 * ServerInit that fits PS2VNC's allocated 1920x1080 backing storage.
 *
 * That widened acceptance is transport-only. It NEVER changes display
 * authority: the complete startup_authoritative_display_profile remains the
 * target throughout the transaction.
 */
static int ps2vnc_startup_connect_reconciled(
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer)
{
    const struct timespec retry_delay = {
        1,
        0
    };

    const ps2vnc_display_profile_t *target;

    unsigned int target_width;
    unsigned int target_height;

    int attempt;

    /*
     * No stored complete profile means the historical first-run/calibration
     * path remains appropriate. Do not widen that unrelated startup case.
     */
    if (!startup_authoritative_display_profile_loaded) {

        if (!startup_display_transaction_loaded ||
            startup_display_transaction.state !=
                PS2VNC_DISPLAY_TX_NONE) {

            screenf("DISPLAY TX: reconcile authority missing");
            return -1;
        }

        return initial_rfb_connect_with_retry(
            rfb_framebuffer,
            gs_framebuffer);
    }

    if (!startup_display_transaction_loaded) {
        screenf("DISPLAY TX: reconcile state invalid");
        return -1;
    }

    target = &startup_authoritative_display_profile;

    /*
     * ps2vnc_startup_apply_authoritative_profile() must have installed the
     * already-selected complete mode before this transport boundary.
     * A disagreement here is a hybrid and remains fatal.
     */
    if (target->mode == NULL ||
        target->mode != active_video_mode) {

        screenf("DISPLAY TX: active mode mismatch");
        return -1;
    }

    target_width = target->logical_width;
    target_height = target->logical_height;

    if (target_width == 0 ||
        target_height == 0 ||
        target_width > VNC_WIDTH ||
        target_height > VNC_HEIGHT) {

        screenf("DISPLAY TX: authority geometry invalid");
        return -1;
    }

    screenf(
        "DISPLAY TX: reconcile %s %ux%u",
        target->mode->name,
        target_width,
        target_height);

    for (attempt = 0;
         attempt < RECONNECT_MAX_ATTEMPTS;
         attempt++) {

        int sock = -1;
        int ok = 1;

        if (!ethGetNetIFLinkStatus()) {
            screenf(
                "Ethernet unavailable - retry %d/%d",
                attempt + 1,
                RECONNECT_MAX_ATTEMPTS);

            nanosleep(&retry_delay, NULL);
            continue;
        }

        reset_rfb_transport_state();

        /*
         * ServerInit's dimensions are unknown until the fresh handshake.
         * Leave the ordinary geometry lock open only across this special
         * bounded recovery boundary.
         */
        desktop_geometry_locked = 0;

        screenf(
            "VNC reconcile attempt %d/%d",
            attempt + 1,
            RECONNECT_MAX_ATTEMPTS);

        sock = rfb_connect_and_handshake_limited(
            VNC_WIDTH,
            VNC_HEIGHT);

        if (sock < 0) {
            /*
             * The failed handshake may have temporarily touched desktop
             * geometry. Reassert the selected authority locally before retry.
             */
            desktop_width = target_width;
            desktop_height = target_height;
            desktop_geometry_locked = 0;

            nanosleep(&retry_delay, NULL);
            continue;
        }

        screenf(
            "DISPLAY TX: server actual %ux%u",
            desktop_width,
            desktop_height);

        /*
         * Same proven sequence used by startup/recovery:
         * RGB565 -> Raw -> one complete framebuffer -> EDS capability probe.
         *
         * The complete Raw receive is safe because the widened handshake is
         * bounded by the actual allocated framebuffer capacity.
         */
        if (set_gs555_pixel_format(sock) < 0)
            ok = 0;

        if (ok &&
            set_raw_encoding(sock) < 0)
            ok = 0;

        if (ok &&
            request_framebuffer(sock, 0) < 0)
            ok = 0;

        if (ok &&
            receive_framebuffer_update(
                sock,
                rfb_framebuffer,
                gs_framebuffer,
                1,
                1) < 0)
            ok = 0;

        if (ok &&
            probe_extended_desktop_size(
                sock,
                rfb_framebuffer,
                gs_framebuffer) < 0)
            ok = 0;

        /*
         * EDS probe above describes the ACTUAL server geometry adopted by the
         * unlocked handshake. Only now may we ask TigerVNC to converge to the
         * complete authoritative profile.
         */
        if (ok &&
            (desktop_width != target_width ||
             desktop_height != target_height)) {

            screenf(
                "DISPLAY TX: resize %ux%u -> %ux%u",
                desktop_width,
                desktop_height,
                target_width,
                target_height);

            if (request_and_confirm_desktop_size(
                    sock,
                    rfb_framebuffer,
                    gs_framebuffer,
                    target_width,
                    target_height) < 0) {

                ok = 0;
            }

            /*
             * request_and_confirm_desktop_size() has now received and
             * validated TigerVNC's reason=1/result=0 EDS acknowledgement.
             *
             * Publish the new PS2-side logical geometry ONLY after that
             * confirmation. This is the no-hybrid commit point for runtime
             * RFB geometry.
             */
            if (ok) {
                desktop_width = target_width;
                desktop_height = target_height;
                desktop_geometry_locked = 1;

                /*
                 * Obtain a clean authoritative framebuffer using the NEW
                 * tightly-packed stride before ordinary startup continues.
                 */
                if (set_raw_encoding(sock) < 0 ||
                    request_framebuffer(sock, 0) < 0 ||
                    receive_framebuffer_update(
                        sock,
                        rfb_framebuffer,
                        gs_framebuffer,
                        1,
                        1) < 0) {

                    ok = 0;
                }
            }

        } else if (ok) {

            /*
             * TigerVNC was already coherent with A. Lock that exact geometry
             * without performing a redundant SetDesktopSize transaction.
             */
            desktop_width = target_width;
            desktop_height = target_height;
            desktop_geometry_locked = 1;
        }

        if (ok) {

            if (desktop_width != target_width ||
                desktop_height != target_height ||
                !desktop_geometry_locked) {

                screenf("DISPLAY TX: final geometry mismatch");
                ok = 0;
            }
        }

        if (ok) {
            screenf(
                "DISPLAY TX: RFB coherent %ux%u",
                desktop_width,
                desktop_height);

            return sock;
        }

        if (sock >= 0)
            close(sock);

        reset_rfb_transport_state();

        /*
         * A failed/partial resize may leave TigerVNC at either geometry.
         * Keep authority A locally and let the next widened handshake discover
         * the server's actual state again.
         */
        desktop_width = target_width;
        desktop_height = target_height;
        desktop_geometry_locked = 0;

        nanosleep(&retry_delay, NULL);
    }

    screenf("DISPLAY TX: unable to reconcile VNC");

    desktop_width = target_width;
    desktop_height = target_height;
    desktop_geometry_locked = 0;

    return -1;
}


/*
 * TEST13J-D4-A4:
 * Persist an accepted interactive display calibration through the
 * dedicated management service on TCP 5959.
 *
 * width/height are the OUTER disappearing-border rectangle used by
 * calibration.  The persistent representation stores the safe interior.
 *
 * Failure is non-fatal.  The accepted calibration remains valid for the
 * current session even if the Pi management service cannot save it.
 */
static int ps2vnc_startup_mode_save_to_pi(
    const ps2vnc_video_mode_t *mode)
{
    struct sockaddr_in server;
    char body[96];
    char request[384];
    unsigned char response[PS2VNC_HTTP_MAX_HEADER];
    unsigned int response_len = 0;
    int body_len;
    int request_len;
    int header_end = -1;
    int sock = -1;
    int result = -1;

    if (mode == NULL ||
        mode->name == NULL ||
        ps2vnc_video_mode_by_name(mode->name) == NULL)
        return -1;

    body_len = snprintf(
        body, sizeof(body),
        "startup_mode = %s\n",
        mode->name
    );

    if (body_len <= 0 ||
        body_len >= (int)sizeof(body))
        return -1;

    request_len = snprintf(
        request, sizeof(request),
        "POST " PS2VNC_DISPLAY_MODE_PATH " HTTP/1.0\r\n"
        "Host: " PS2VNC_BOOTSTRAP_PI_IP "\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        body_len
    );

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
            sock, request, request_len) != request_len)
        goto done;

    if (send_exact(
            sock, body, body_len) != body_len)
        goto done;

    while (response_len < sizeof(response)) {
        int n = recv(
            sock,
            response + response_len,
            (int)(sizeof(response) - response_len),
            0
        );

        if (n <= 0)
            break;

        response_len += (unsigned int)n;

        header_end = ps2vnc_http_find_header_end(
            response, response_len
        );

        if (header_end >= 0)
            break;
    }

    if (header_end < 0 ||
        (unsigned int)header_end > PS2VNC_HTTP_MAX_HEADER)
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


static int ps2vnc_display_save_to_pi(
    int width,
    int height,
    int offset_x,
    int offset_y)
{
    struct sockaddr_in server;

    char body[160];
    char request[384];

    unsigned char response[
        PS2VNC_HTTP_MAX_HEADER
    ];

    int safe_width;
    int safe_height;

    int body_len;
    int request_len;

    unsigned int response_len = 0;

    int header_end = -1;
    int sock = -1;
    int result = -1;

    if (!screen_fit_geometry_valid(
            width,
            height,
            offset_x,
            offset_y)) {

        return -1;
    }

    /*
     * D17AL-F1:
     * Persist exactly what the calibration rectangle selected.
     */
    safe_width = width;
    safe_height = height;

    if (active_video_mode == NULL ||
        active_video_mode->name == NULL)
        return -1;

    body_len = snprintf(
        body,
        sizeof(body),
        "mode = %s\n"
        "safe_width = %d\n"
        "safe_height = %d\n"
        "safe_x = %d\n"
        "safe_y = %d\n",
        active_video_mode->name,
        safe_width,
        safe_height,
        offset_x,
        offset_y
    );

    if (body_len <= 0 ||
        body_len >= (int)sizeof(body)) {

        return -1;
    }

    request_len = snprintf(
        request,
        sizeof(request),
        "POST " PS2VNC_DISPLAY_PATH " HTTP/1.0\r\n"
        "Host: " PS2VNC_BOOTSTRAP_PI_IP "\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        body_len
    );

    if (request_len <= 0 ||
        request_len >= (int)sizeof(request)) {

        return -1;
    }

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
            sizeof(server)) < 0) {

        goto done;
    }

    if (send_exact(
            sock,
            request,
            request_len) != request_len) {

        goto done;
    }

    if (send_exact(
            sock,
            body,
            body_len) != body_len) {

        goto done;
    }

    /*
     * We need only the response header.  Stop as soon as it is complete
     * rather than depending on the response body or connection shutdown.
     */
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

    if (header_end < 0)
        goto done;

    if ((unsigned int)header_end >
        PS2VNC_HTTP_MAX_HEADER) {

        goto done;
    }

    if (!ps2vnc_http_status_is_200(
            response,
            (unsigned int)header_end)) {

        goto done;
    }

    result = 0;

done:
    if (sock >= 0)
        close(sock);

    return result;
}


/*
 * TEST15E2-D1:
 * Read and validate one mode-specific preset without applying it to the
 * current screen-fit state.  This is safe to call before a runtime switch.
 */
static int ps2vnc_config_get_display_for_mode(
    const ps2vnc_video_mode_t *mode,
    ps2vnc_display_config_t *out_config)
{
    unsigned char *text;
    unsigned int len = 0;
    int result;

    if (mode == NULL || out_config == NULL)
        return 0;

    text = (unsigned char *)malloc(PS2VNC_CONFIG_MAX_BYTES);
    if (text == NULL)
        return 0;

    if (ps2vnc_management_get(
            PS2VNC_CONFIG_PATH,
            text, PS2VNC_CONFIG_MAX_BYTES, &len) < 0) {
        free(text);
        return 0;
    }

    result = ps2vnc_config_parse_display_for_mode(
        (const char *)text, len, mode, out_config
    );

    free(text);
    return result == PS2VNC_CONFIG_OK;
}


/*
 * Convert a stored safe-desktop preset into the physical presentation
 * rectangle for its own video mode without changing active_video_mode or
 * global screen-fit state.
 */

static int ps2vnc_config_presented_geometry_for_mode(
    const ps2vnc_video_mode_t *mode,
    const ps2vnc_display_config_t *config,
    unsigned int *out_x,
    unsigned int *out_y,
    unsigned int *out_w,
    unsigned int *out_h)
{
    int safe_x;
    int safe_y;
    unsigned int width;
    unsigned int height;

    if (mode == NULL || config == NULL ||
        out_x == NULL || out_y == NULL ||
        out_w == NULL || out_h == NULL)
        return -1;

    if (!screen_fit_geometry_valid_for_mode(
            mode,
            config->safe_width,
            config->safe_height,
            config->safe_x,
            config->safe_y))
        return -1;

    safe_x =
        ((int)mode->raster_width - config->safe_width) / 2 +
        config->safe_x;

    safe_y =
        ((int)mode->raster_height - config->safe_height) / 2 +
        config->safe_y;

    if (safe_x < 0 || safe_y < 0)
        return -1;

    width =
        ((unsigned int)config->safe_width < VNC_WIDTH) ?
        (unsigned int)config->safe_width : VNC_WIDTH;

    height =
        ((unsigned int)config->safe_height < VNC_HEIGHT) ?
        (unsigned int)config->safe_height : VNC_HEIGHT;

    *out_x =
        (unsigned int)safe_x +
        ((unsigned int)config->safe_width - width) / 2;

    *out_y =
        (unsigned int)safe_y +
        ((unsigned int)config->safe_height - height) / 2;

    *out_w = width;
    *out_h = height;

    return region_fits_u32(
        *out_x,
        *out_y,
        *out_w,
        *out_h,
        mode->raster_width,
        mode->raster_height
    ) ? 0 : -1;
}



static int calculate_presented_desktop_geometry(
    unsigned int *out_x,
    unsigned int *out_y,
    unsigned int *out_w,
    unsigned int *out_h);


/*
 * H4C3C3B3:
 * Install the already-selected complete startup display authority locally.
 *
 * Authority was selected earlier from durable transaction state. This
 * function does NOT choose between A and B. It only makes local startup
 * configuration conform to that already-made decision.
 *
 * No individual mode/profile fields are allowed to come from unrelated
 * sources.
 */
static int ps2vnc_startup_apply_authoritative_profile(void)
{
    const ps2vnc_display_profile_t *target;

    ps2vnc_display_config_t target_config;
    ps2vnc_display_profile_t config_profile;

    ps2vnc_backend_t expected_backend;

    unsigned int check_x;
    unsigned int check_y;
    unsigned int check_w;
    unsigned int check_h;

    /*
     * No stored preset + no transaction retains the historical
     * first-run/calibration startup path.
     */
    if (!startup_authoritative_display_profile_loaded) {

        if (!startup_display_transaction_loaded ||
            startup_display_transaction.state !=
                PS2VNC_DISPLAY_TX_NONE) {

            screenf("DISPLAY TX: missing pending authority");
            return -1;
        }

        return 0;
    }

    if (!startup_display_transaction_loaded)
        return -1;

    target = &startup_authoritative_display_profile;

    if (!ps2vnc_display_profile_valid(target)) {
        screenf("DISPLAY TX: startup profile invalid");
        return -1;
    }

    /*
     * GS creation independently selects the mode's default backend.
     * Therefore the durable profile must claim exactly that same backend
     * before active_video_mode is changed.
     */
    expected_backend =
        ps2vnc_video_mode_default_backend(target->mode);

    if (expected_backend == PS2VNC_BACKEND_NONE ||
        target->backend != expected_backend) {

        screenf("DISPLAY TX: backend/profile mismatch");
        return -1;
    }

    /*
     * Usually the startup config already describes the authoritative
     * profile:
     *
     *   NONE                 -> confirmed profile
     *   PROVISIONAL/RESTORE  -> previous A
     *   COMMITTING after the config write -> candidate B
     *
     * The special interrupted-COMMITTING window can instead have:
     *
     *   startup config = complete A
     *   durable authority = complete B
     *
     * In that one case, load B's existing mode-specific calibration,
     * derive a complete profile from it WITHOUT mutating state, and require
     * exact equality with durable candidate B before applying it.
     */
    if (!startup_confirmed_display_profile_loaded ||
        !ps2vnc_display_tx_profiles_equal(
            &startup_confirmed_display_profile,
            target)) {

        if (startup_display_transaction.state !=
                PS2VNC_DISPLAY_TX_COMMITTING ||
            !startup_confirmed_display_profile_loaded ||
            !ps2vnc_display_tx_profiles_equal(
                &startup_confirmed_display_profile,
                &startup_display_transaction.previous)) {

            screenf("DISPLAY TX: authority/config mismatch");
            return -1;
        }

        if (!ps2vnc_config_get_display_for_mode(
                target->mode,
                &target_config)) {

            screenf("DISPLAY TX: candidate preset unavailable");
            return -1;
        }

        if (ps2vnc_config_presented_geometry_for_mode(
                target->mode,
                &target_config,
                &check_x,
                &check_y,
                &check_w,
                &check_h) < 0 ||
            ps2vnc_display_profile_resolve_candidate(
                target->mode,
                check_x,
                check_y,
                check_w,
                check_h,
                &config_profile) < 0) {

            screenf("DISPLAY TX: candidate preset invalid");
            return -1;
        }

        if (!ps2vnc_display_tx_profiles_equal(
                &config_profile,
                target)) {

            screenf("DISPLAY TX: candidate preset mismatch");
            return -1;
        }

        /*
         * screen_fit_apply_preset() validates against active_video_mode,
         * so change the mode only AFTER the complete B preset has been
         * independently derived and proven equal to durable B.
         */
        active_video_mode = target->mode;

        if (!ps2vnc_config_apply_display(&target_config)) {
            screenf("DISPLAY TX: candidate preset apply failed");
            return -1;
        }

    } else {

        /*
         * The loaded preset is already exactly the authoritative complete
         * profile. Only select its mode; screen-fit state already belongs
         * to this same profile.
         */
        active_video_mode = target->mode;
    }

    /*
     * Prove that the local screen-fit state now reconstructs the exact
     * physical presentation rectangle stored in the complete authority.
     *
     * Do this before RFB or GS initialization.
     */
    if (calculate_presented_desktop_geometry(
            &check_x,
            &check_y,
            &check_w,
            &check_h) < 0 ||
        check_x != target->output_x ||
        check_y != target->output_y ||
        check_w != target->output_width ||
        check_h != target->output_height) {

        screenf("DISPLAY TX: local presentation mismatch");
        return -1;
    }

    screenf(
        "DISPLAY TX: local authority %s/%s",
        target->mode->name,
        ps2vnc_backend_transaction_name(target->backend));

    return 0;
}



static int ps2vnc_config_load_from_pi(void)
{
    unsigned char *config_text;

    unsigned int config_len = 0;

    ps2vnc_display_config_t display;
    const ps2vnc_video_mode_t *startup_mode = NULL;

    unsigned int profile_x;
    unsigned int profile_y;
    unsigned int profile_w;
    unsigned int profile_h;

    int startup_result;
    int parse_result;

    ps2vnc_display_config_loaded = 0;
    startup_confirmed_display_profile_loaded = 0;

    memset(
        &startup_confirmed_display_profile,
        0,
        sizeof(startup_confirmed_display_profile));

    config_text = (unsigned char *)malloc(
        PS2VNC_CONFIG_MAX_BYTES
    );

    if (config_text == NULL) {
        screenf("CONFIG: buffer allocation failed");
        return 0;
    }

    if (ps2vnc_management_get(
            PS2VNC_CONFIG_PATH,
            config_text,
            PS2VNC_CONFIG_MAX_BYTES,
            &config_len) < 0) {

        free(config_text);

        screenf("CONFIG: using calibration");
        return 0;
    }

    screenf(
        "CONFIG: fetched %u bytes",
        config_len
    );

    /*
     * Startup selection and calibration are deliberately separate.
     *
     * A missing or malformed startup_mode never prevents PS2VNC from
     * starting.  The compiled 1080i mode remains the fallback.  When a
     * valid symbolic mode is present, select it before interpreting any
     * mode-specific calibration.
     */
    startup_result =
        ps2vnc_config_parse_startup_mode(
            (const char *)config_text,
            config_len,
            &startup_mode
        );

    if (startup_result > 0) {
        active_video_mode = startup_mode;

        screenf(
            "CONFIG: startup mode %s",
            active_video_mode->name
        );

    } else if (startup_result < 0) {

        screenf("CONFIG: invalid startup mode");
        screenf(
            "CONFIG: using default mode %s",
            active_video_mode->name
        );

    } else {

        screenf(
            "CONFIG: no startup mode; default %s",
            active_video_mode->name
        );
    }

    parse_result =
        ps2vnc_config_parse_display_for_mode(
            (const char *)config_text,
            config_len,
            active_video_mode,
            &display
        );

    if (parse_result ==
            PS2VNC_CONFIG_NO_DISPLAY) {

        free(config_text);

        screenf("CONFIG: no display preset");
        return 0;
    }

    if (parse_result ==
            PS2VNC_CONFIG_INVALID_TEXT) {

        free(config_text);

        screenf("CONFIG: invalid display text");
        screenf("CONFIG: using calibration");
        return 0;
    }

    if (parse_result ==
            PS2VNC_CONFIG_INVALID_GEOMETRY) {

        free(config_text);

        screenf("CONFIG: invalid geometry");
        screenf("CONFIG: using calibration");
        return 0;
    }

    if (parse_result !=
            PS2VNC_CONFIG_OK) {

        free(config_text);

        screenf("CONFIG: unknown parser result");
        screenf("CONFIG: using calibration");
        return 0;
    }

    free(config_text);

    if (!ps2vnc_config_apply_display(
            &display)) {

        screenf("CONFIG: preset apply rejected");
        screenf("CONFIG: using calibration");
        return 0;
    }

    /*
     * Convert the stored safe-area calibration into the exact coherent
     * H4 display profile before discarding startup configuration context.
     *
     * At this point no GS backend has been constructed yet, so use the
     * mode's normal backend resolution rather than capture_active().
     */
    if (ps2vnc_config_presented_geometry_for_mode(
            active_video_mode,
            &display,
            &profile_x,
            &profile_y,
            &profile_w,
            &profile_h) < 0 ||
        ps2vnc_display_profile_resolve_candidate(
            active_video_mode,
            profile_x,
            profile_y,
            profile_w,
            profile_h,
            &startup_confirmed_display_profile) < 0) {

        screenf("CONFIG: confirmed profile invalid");
        screenf("CONFIG: using calibration");
        return 0;
    }

    startup_confirmed_display_profile_loaded = 1;
    ps2vnc_display_config_loaded = 1;

    screenf(
        "CONFIG: display %dx%d %d,%d",
        display.safe_width,
        display.safe_height,
        display.safe_x,
        display.safe_y
    );

    screenf("CONFIG: display preset loaded");

    return 1;
}


/*
 * D4-A4 geometry diagnostic.
 *
 * Emit one concise snapshot after calibrated geometry has been converted
 * into the physical 1280x720 output position.
 */
static void debug_emit_geometry_snapshot(void)
{
    char msg[192];
    int len;

    if (debug_udp_sock < 0)
        return;

    len = snprintf(
        msg,
        sizeof(msg),
        "GEOM fit=%dx%d@%d,%d full=%d "
        "desk=%ux%u out=%u,%u cfg=%d force=%d",
        screen_fit_width,
        screen_fit_height,
        screen_fit_offset_x,
        screen_fit_offset_y,
        screen_fit_full_bypass,
        desktop_width,
        desktop_height,
        desktop_output_x,
        desktop_output_y,
        ps2vnc_display_config_loaded,
        force_screen_fit_calibration
    );

    if (len <= 0)
        return;

    if (len >= (int)sizeof(msg))
        len = sizeof(msg) - 1;

    sendto(
        debug_udp_sock,
        msg,
        len,
        0,
        (struct sockaddr *)&debug_udp_addr,
        sizeof(debug_udp_addr)
    );
}


/*
 * Interactive calibration policy.
 *
 * Size and position are constrained together against the physical
 * output coordinate space.
 */
static void screen_fit_clamp_interactive(
    int *width,
    int *height,
    int *offset_x,
    int *offset_y)
{
    int remaining_x;
    int remaining_y;
    int centered_x;
    int centered_y;
    int max_positive_x;
    int max_positive_y;

    if (*width < SCREEN_FIT_MIN_WIDTH)
        *width = SCREEN_FIT_MIN_WIDTH;

    if (*width > OUTPUT_WIDTH)
        *width = OUTPUT_WIDTH;

    if (*height < SCREEN_FIT_MIN_HEIGHT)
        *height = SCREEN_FIT_MIN_HEIGHT;

    if (*height > OUTPUT_HEIGHT)
        *height = OUTPUT_HEIGHT;

    remaining_x = OUTPUT_WIDTH - *width;
    remaining_y = OUTPUT_HEIGHT - *height;

    centered_x = remaining_x / 2;
    centered_y = remaining_y / 2;

    max_positive_x = remaining_x - centered_x;
    max_positive_y = remaining_y - centered_y;

    if (*offset_x < -centered_x)
        *offset_x = -centered_x;

    if (*offset_x > max_positive_x)
        *offset_x = max_positive_x;

    if (*offset_y < -centered_y)
        *offset_y = -centered_y;

    if (*offset_y > max_positive_y)
        *offset_y = max_positive_y;
}


/*
 * Draw text directly with the existing 5x7 OSK font.
 * Calibration is temporary, so a few tiny GS sprites per glyph are
 * preferable to allocating another texture or consuming more VRAM.
 */
static float screen_fit_draw_x(int x)
{
    return ps2vnc_display_output_x((float)x);
}

static float screen_fit_draw_y(int y)
{
    return ps2vnc_display_output_y((float)y);
}

static void screen_fit_prim_sprite(
    GSGLOBAL *g, int x0, int y0, int x1, int y1, int z, u64 color)
{
    gsKit_prim_sprite(g,
        screen_fit_draw_x(x0), screen_fit_draw_y(y0),
        screen_fit_draw_x(x1), screen_fit_draw_y(y1), z, color);
}


/*
 * D17AL-F5:
 *
 * Calibration decoration may intentionally extend beyond the physical
 * raster while the selected SAFE DESKTOP remains fully in-bounds.
 *
 * Clip that decoration here before applying the backend coordinate
 * transform. Negative/out-of-range visualization coordinates therefore
 * never reach gsKit/GS.
 */
static void screen_fit_prim_sprite_clipped(
    GSGLOBAL *g,
    int x0,
    int y0,
    int x1,
    int y1,
    int z,
    u64 color)
{
    if (x0 < 0)
        x0 = 0;

    if (y0 < 0)
        y0 = 0;

    if (x1 > (int)OUTPUT_WIDTH)
        x1 = (int)OUTPUT_WIDTH;

    if (y1 > (int)OUTPUT_HEIGHT)
        y1 = (int)OUTPUT_HEIGHT;

    if (x0 >= x1 || y0 >= y1)
        return;

    screen_fit_prim_sprite(
        g,
        x0,
        y0,
        x1,
        y1,
        z,
        color
    );
}

static void screen_fit_draw_char(
    GSGLOBAL *gsGlobal,
    int x,
    int y,
    char c,
    int scale,
    u64 color)
{
    const unsigned char *glyph = osk_find_glyph(c);
    int gy;
    int gx;

    if (glyph == NULL)
        return;

    for (gy = 0; gy < 7; gy++) {
        for (gx = 0; gx < 5; gx++) {
            if (glyph[gy] & (1 << (4 - gx))) {
                screen_fit_prim_sprite(
                    gsGlobal,
                    (float)(x + gx * scale),
                    (float)(y + gy * scale),
                    (float)(x + (gx + 1) * scale),
                    (float)(y + (gy + 1) * scale),
                    3,
                    color
                );
            }
        }
    }
}


static void screen_fit_draw_text_centered(
    GSGLOBAL *gsGlobal,
    int y,
    const char *text,
    int scale,
    u64 color)
{
    int len = (int)strlen(text);
    int char_w = 6 * scale;
    int text_w = len * char_w - scale;
    int x = (OUTPUT_WIDTH - text_w) / 2;
    int i;

    for (i = 0; i < len; i++) {
        screen_fit_draw_char(
            gsGlobal,
            x + i * char_w,
            y,
            text[i],
            scale,
            color
        );
    }
}


/*
 * TEST13J-D3-A3:
 * Draw a square centered on one theoretical calibration corner.
 *
 * The square is split diagonally into two right triangles:
 *
 *   bright = toward the usable desktop
 *   gray   = toward the hidden/outside area
 *
 * Since the marker straddles the calibration corner, comparing the
 * visible pieces at all four corners gives an immediate indication
 * that the hidden border is positioned evenly.
 */
static void screen_fit_draw_corner_marker(
    GSGLOBAL *gsGlobal,
    int corner_x,
    int corner_y,
    int sx,
    int sy,
    u64 corner_color,
    u64 inner_color)
{
    const int size = SCREEN_FIT_CORNER_SIZE;
    int x_far;
    int y_far;
    int x_min;
    int x_max;
    int y_min;
    int y_max;
    int v;

    /*
     * TEST13J-D3-A3:
     *
     * corner_x/corner_y is the INSIDE corner of the white border.
     * The complete marker therefore lies inside the usable rectangle
     * and merely touches the border; it never overlaps it.
     *
     * sx/sy point inward:
     *
     *   TL  +1,+1
     *   TR  -1,+1
     *   BL  +1,-1
     *   BR  -1,-1
     */

    x_far = corner_x + sx * size;
    y_far = corner_y + sy * size;

    x_min = (corner_x < x_far) ? corner_x : x_far;
    x_max = (corner_x > x_far) ? corner_x : x_far;

    y_min = (corner_y < y_far) ? corner_y : y_far;
    y_max = (corner_y > y_far) ? corner_y : y_far;

    /*
     * First draw the complete inward square in cyan.
     */
    screen_fit_prim_sprite(
        gsGlobal,
        (float)x_min,
        (float)y_min,
        (float)x_max,
        (float)y_max,
        3,
        inner_color
    );

    /*
     * Then construct the amber corner triangle from one-pixel strips.
     *
     * This avoids gsKit triangle winding/culling entirely.
     *
     * At the actual border corner the amber wedge is full width.
     * It decreases by one pixel per row until the diagonal reaches
     * the opposite side of the square.
     */
    for (v = 0; v < size; v++) {
        int run = size - v;

        int ya;
        int yb;

        int xa;
        int xb;

        if (sy > 0) {
            ya = corner_y + v;
            yb = ya + 1;
        } else {
            yb = corner_y - v;
            ya = yb - 1;
        }

        if (sx > 0) {
            xa = corner_x;
            xb = corner_x + run;
        } else {
            xb = corner_x;
            xa = corner_x - run;
        }

        screen_fit_prim_sprite(
            gsGlobal,
            (float)xa,
            (float)ya,
            (float)xb,
            (float)yb,
            4,
            corner_color
        );
    }
}

static int draw_screen_fit_calibration(
    GSGLOBAL *gsGlobal,
    int width,
    int height,
    int offset_x,
    int offset_y,
    u64 black)
{
    const u64 white =
        GS_SETREG_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x00);

    const u64 marker_amber =
        GS_SETREG_RGBAQ(0x80, 0x63, 0x0C, 0x80, 0x00);

    const u64 marker_cyan =
        GS_SETREG_RGBAQ(0x10, 0x70, 0x80, 0x80, 0x00);

    int x0 =
        (OUTPUT_WIDTH - width) / 2 + offset_x;

    int y0 =
        (OUTPUT_HEIGHT - height) / 2 + offset_y;

    int x1 = x0 + width;
    int y1 = y0 + height;

    /*
     * Black background.
     */
    screen_fit_prim_sprite(
        gsGlobal,
        0.0f,
        0.0f,
        (float)OUTPUT_WIDTH,
        (float)OUTPUT_HEIGHT,
        1,
        black
    );

    /*
     * D17AL-F5:
     *
     * x0/y0/x1/y1 are the exact SAFE DESKTOP edges.
     *
     * SCREEN_FIT_BORDER is visualization only, so draw it immediately
     * OUTSIDE those edges rather than consuming safe pixels inside them.
     *
     * When an edge of the safe desktop reaches the physical raster edge,
     * the corresponding border clips completely away. This is precisely
     * the calibration indication we want.
     */
    screen_fit_prim_sprite_clipped(
        gsGlobal,
        x0 - SCREEN_FIT_BORDER,
        y0 - SCREEN_FIT_BORDER,
        x1 + SCREEN_FIT_BORDER,
        y0,
        2,
        white
    );

    screen_fit_prim_sprite_clipped(
        gsGlobal,
        x0 - SCREEN_FIT_BORDER,
        y1,
        x1 + SCREEN_FIT_BORDER,
        y1 + SCREEN_FIT_BORDER,
        2,
        white
    );

    screen_fit_prim_sprite_clipped(
        gsGlobal,
        x0 - SCREEN_FIT_BORDER,
        y0,
        x0,
        y1,
        2,
        white
    );

    screen_fit_prim_sprite_clipped(
        gsGlobal,
        x1,
        y0,
        x1 + SCREEN_FIT_BORDER,
        y1,
        2,
        white
    );

    /*
     * TEST13J-D3-A3:
     * Corner clipping references.
     *
     * Each 28x28 square straddles one theoretical border corner and is
     * divided into two mirrored right triangles. When the border itself
     * is just outside the visible area, compare how much of these four
     * markers remains visible.
     */
    /*
     * Four high-contrast clipping gauges.
     *
     * Anchor them at the INNER edges of the white calibration border.
     * Nothing in these markers overlaps the border itself.
     */

    /* Top-left */
    screen_fit_draw_corner_marker(
        gsGlobal,
        x0,
        y0,
        1, 1,
        marker_amber,
        marker_cyan
    );

    /* Top-right */
    screen_fit_draw_corner_marker(
        gsGlobal,
        x1,
        y0,
        -1, 1,
        marker_amber,
        marker_cyan
    );

    /* Bottom-left */
    screen_fit_draw_corner_marker(
        gsGlobal,
        x0,
        y1,
        1, -1,
        marker_amber,
        marker_cyan
    );

    /* Bottom-right */
    screen_fit_draw_corner_marker(
        gsGlobal,
        x1,
        y1,
        -1, -1,
        marker_amber,
        marker_cyan
    );

    /*
     * Keep all instructions near the middle so overscan cannot hide them.
     */
    screen_fit_draw_text_centered(
        gsGlobal,
        OUTPUT_HEIGHT / 2 - 90,
        "SCREEN FIT",
        3,
        white
    );

    screen_fit_draw_text_centered(
        gsGlobal,
        OUTPUT_HEIGHT / 2 - 40,
        "DPAD: SIZE     R1+DPAD: MOVE",
        2,
        white
    );

    screen_fit_draw_text_centered(
        gsGlobal,
        OUTPUT_HEIGHT / 2 - 10,
        "TAP: 1 PIXEL     HOLD: FAST",
        2,
        white
    );

    screen_fit_draw_text_centered(
        gsGlobal,
        OUTPUT_HEIGHT / 2 + 20,
        "HIDE BORDER - MATCH CORNER TRIANGLES",
        2,
        white
    );

    screen_fit_draw_text_centered(
        gsGlobal,
        OUTPUT_HEIGHT / 2 + 50,
        "X: SAVE   TRIANGLE: RESET   CIRCLE: FULL",
        2,
        white
    );

    if (ps2vnc_display_finish_draw(gsGlobal) < 0)
        return -1;

    if (ps2vnc_display_flip(gsGlobal) < 0)
        return -1;

    return 0;
}


static int run_screen_fit_calibration(
    GSGLOBAL *gsGlobal,
    u64 black,
    int save_on_accept,
    int *accepted_out)
{
    struct padButtonStatus buttons;

    unsigned int pressed = 0;
    unsigned int last_pressed = 0;
    unsigned int changed = 0;

    unsigned int hold_action = 0;
    unsigned int hold_ticks = 0;

    /*
     * Start forced recalibration from the exact persistent geometry when
     * one was loaded.  screen_fit_* already describes the validated OUTER
     * disappearing-border rectangle and its center-relative position.
     *
     * With no stored display preset, preserve the original first-run
     * behavior by reconstructing the OUTER rectangle from the current
     * TigerVNC safe-interior dimensions and starting centered.
     */
    int width;
    int height;
    int offset_x;
    int offset_y;

    int input_armed = 0;

    if (accepted_out != NULL)
        *accepted_out = 0;

    if (ps2vnc_display_config_loaded) {
        width = screen_fit_width;
        height = screen_fit_height;
        offset_x = screen_fit_offset_x;
        offset_y = screen_fit_offset_y;

    } else {
        offset_x = 0;
        offset_y = 0;

        /*
         * Reconstruct the OUTER calibration rectangle without trusting
         * runtime dimensions enough to perform an unchecked
         * addition/conversion.
         */
        if (desktop_width >
                (unsigned int)OUTPUT_WIDTH) {

            width = OUTPUT_WIDTH;

        } else {
            width =
                (int)desktop_width;
        }

        if (desktop_height >
                (unsigned int)OUTPUT_HEIGHT) {

            height = OUTPUT_HEIGHT;

        } else {
            height =
                (int)desktop_height;
        }
    }

    screen_fit_clamp_interactive(
        &width,
        &height,
        &offset_x,
        &offset_y
    );

    for (;;) {
        if (draw_screen_fit_calibration(
                gsGlobal,
                width,
                height,
                offset_x,
                offset_y,
                black) < 0)
            return -1;

        if (padGetState(0, 0) != PAD_STATE_STABLE)
            continue;

        if (padRead(0, 0, &buttons) == 0)
            continue;

        pressed = 0xFFFFu ^ (unsigned int)buttons.btns;

        /*
         * Require a clean controller release after ELF launch.
         */
        if (!input_armed) {
            if (pressed == 0) {
                input_armed = 1;
                last_pressed = 0;
            }
            continue;
        }

        changed = pressed ^ last_pressed;

        /*
         * X saves this calibration.
         */
        if ((changed & PAD_CROSS) &&
            (pressed & PAD_CROSS)) {

            /*
             * Interactive values are already clamped, but saving still
             * passes through the authoritative validation boundary.
             */
            if (screen_fit_apply_preset(
                    width,
                    height,
                    offset_x,
                    offset_y,
                    0)) {

                if (accepted_out != NULL)
                    *accepted_out = 1;

                /*
                 * Startup calibration retains its proven immediate-save
                 * behavior. Runtime calibration passes save_on_accept=0 and
                 * persists only after its live geometry transaction succeeds.
                 */
                if (save_on_accept) {
                    if (ps2vnc_display_save_to_pi(
                            width,
                            height,
                            offset_x,
                            offset_y) < 0) {

                        screenf(
                            "CONFIG: display save failed"
                        );

                    } else {
                        screenf(
                            "CONFIG: display saved"
                        );
                    }
                }
            }

            return 0;
        }

        /*
         * Circle bypasses calibration.
         */
        if ((changed & PAD_CIRCLE) &&
            (pressed & PAD_CIRCLE)) {

            screen_fit_apply_preset(
                OUTPUT_WIDTH,
                OUTPUT_HEIGHT,
                0,
                0,
                1
            );
            return 0;
        }

        /*
         * Triangle resets size and position.
         */
        if ((changed & PAD_TRIANGLE) &&
            (pressed & PAD_TRIANGLE)) {

            width = SCREEN_FIT_START_WIDTH;
            height = SCREEN_FIT_START_HEIGHT;
            offset_x = 0;
            offset_y = 0;

            hold_action = 0;
            hold_ticks = 0;

        } else {
            unsigned int dpad =
                pressed &
                (PAD_LEFT | PAD_RIGHT |
                 PAD_UP | PAD_DOWN);

            unsigned int action = dpad;
            int do_step = 0;
            int step = SCREEN_FIT_FINE_STEP;
            int move_mode =
                ((pressed & PAD_R1) != 0);

            /*
             * Include the modifier in the held action so pressing or
             * releasing R1 while holding the D-pad immediately changes
             * between SIZE and MOVE behavior.
             */
            if (dpad != 0 && move_mode)
                action |= SCREEN_FIT_ACTION_MOVE;

            if (action != 0 &&
                action != hold_action) {

                hold_action = action;
                hold_ticks = 0;
                do_step = 1;

            } else if (action == 0) {

                hold_action = 0;
                hold_ticks = 0;

            } else {
                hold_ticks++;

                if (hold_ticks >=
                        SCREEN_FIT_REPEAT_DELAY_POLLS &&
                    ((hold_ticks -
                      SCREEN_FIT_REPEAT_DELAY_POLLS) %
                     SCREEN_FIT_REPEAT_POLLS) == 0) {

                    do_step = 1;
                    step = SCREEN_FIT_FAST_STEP;
                }
            }

            if (do_step) {
                if (move_mode) {
                    /*
                     * R1 + D-pad moves the complete rectangle.
                     */
                    if (dpad & PAD_LEFT)
                        offset_x -= step;

                    if (dpad & PAD_RIGHT)
                        offset_x += step;

                    if (dpad & PAD_UP)
                        offset_y -= step;

                    if (dpad & PAD_DOWN)
                        offset_y += step;

                } else {
                    /*
                     * Plain D-pad changes size symmetrically around
                     * the currently selected center.
                     */
                    if (dpad & PAD_LEFT)
                        width -= step;

                    if (dpad & PAD_RIGHT)
                        width += step;

                    if (dpad & PAD_UP)
                        height += step;

                    if (dpad & PAD_DOWN)
                        height -= step;
                }

                /*
                 * Width, height and signed position are one geometry.
                 */
                screen_fit_clamp_interactive(
                    &width,
                    &height,
                    &offset_x,
                    &offset_y
                );
            }
        }

        last_pressed = pressed;
    }
}



/*
 * TEST13J-D3-A2:
 * Convert the visible calibration result into the actual remote desktop.
 *
 * X calibration:
 *     white border is deliberately pushed outside the visible TV area;
 *     use the interior of that border as guaranteed-safe pixels.
 *
 * Circle:
 *     deliberate full 1280x720 bypass.
 */

static int calculate_safe_desktop_geometry(
    unsigned int *out_x,
    unsigned int *out_y,
    unsigned int *out_w,
    unsigned int *out_h)
{
    int safe_x;
    int safe_y;

    if (screen_fit_full_bypass) {
        *out_x = 0;
        *out_y = 0;
        *out_w = OUTPUT_WIDTH;
        *out_h = OUTPUT_HEIGHT;
        return 0;
    }

    if (!screen_fit_geometry_valid(
            screen_fit_width,
            screen_fit_height,
            screen_fit_offset_x,
            screen_fit_offset_y))
        return -1;

    safe_x =
        (OUTPUT_WIDTH - screen_fit_width) / 2 +
        screen_fit_offset_x;

    safe_y =
        (OUTPUT_HEIGHT - screen_fit_height) / 2 +
        screen_fit_offset_y;

    if (safe_x < 0 || safe_y < 0)
        return -1;

    if (!region_fits_u32(
            (unsigned int)safe_x,
            (unsigned int)safe_y,
            (unsigned int)screen_fit_width,
            (unsigned int)screen_fit_height,
            OUTPUT_WIDTH,
            OUTPUT_HEIGHT))
        return -1;

    *out_x = (unsigned int)safe_x;
    *out_y = (unsigned int)safe_y;
    *out_w = (unsigned int)screen_fit_width;
    *out_h = (unsigned int)screen_fit_height;

    return 0;
}



/*
 * Physical screen calibration and RFB desktop capacity are independent.
 *
 * Preserve the historical behavior when the calibrated safe area is
 * smaller than the RFB capacity.  When it is larger, retain the maximum
 * supported RFB desktop and center it inside the physical safe region.
 */
static int calculate_presented_desktop_geometry(
    unsigned int *out_x,
    unsigned int *out_y,
    unsigned int *out_w,
    unsigned int *out_h)
{
    unsigned int safe_x;
    unsigned int safe_y;
    unsigned int safe_w;
    unsigned int safe_h;
    unsigned int width;
    unsigned int height;

    if (calculate_safe_desktop_geometry(
            &safe_x, &safe_y, &safe_w, &safe_h) < 0)
        return -1;

    width = (safe_w < VNC_WIDTH) ? safe_w : VNC_WIDTH;
    height = (safe_h < VNC_HEIGHT) ? safe_h : VNC_HEIGHT;

    *out_x = safe_x + ((safe_w - width) / 2);
    *out_y = safe_y + ((safe_h - height) / 2);
    *out_w = width;
    *out_h = height;

    return region_fits_u32(
        *out_x, *out_y, *out_w, *out_h,
        OUTPUT_WIDTH, OUTPUT_HEIGHT) ? 0 : -1;
}



/*
 * G2-D2A:
 * Temporarily transfer libpad ownership to the main/render thread.
 */
static int ps2vnc_controller_pad_acquire(void)
{
    const struct timespec delay = { 0, 10000000 };
    int i;

    controller_pad_pause_requested = 1;

    for (i = 0; i < 200; i++) {
        if (controller_pad_pause_ack)
            return 0;

        if (controller_thread_error || exit_requested)
            break;

        nanosleep(&delay, NULL);
    }

    controller_pad_pause_requested = 0;
    return -1;
}


/*
 * The controller must not resume until every calibration button has been
 * physically released. This prevents the accepting X/Circle/D-pad state
 * from becoming remote-desktop input after ownership returns.
 */
static int ps2vnc_controller_pad_wait_release(void)
{
    const struct timespec delay = { 0, 10000000 };
    struct padButtonStatus buttons;

    for (;;) {
        if (padGetState(0, 0) == PAD_STATE_STABLE &&
            padRead(0, 0, &buttons) != 0) {

            unsigned int pressed =
                0xFFFFu ^ (unsigned int)buttons.btns;

            if (pressed == 0)
                return 0;
        }

        nanosleep(&delay, NULL);
    }
}


static int ps2vnc_controller_pad_release(void)
{
    const struct timespec delay = { 0, 10000000 };
    int i;

    controller_pad_pause_requested = 0;

    for (i = 0; i < 200; i++) {
        if (!controller_pad_pause_ack)
            return 0;

        nanosleep(&delay, NULL);
    }

    return -1;
}


/*
 * Rebuild the RFB desktop at a newly calibrated geometry without changing
 * the electrical video mode or reconstructing the GS display.
 *
 * HIRES is geometry-independent at the allocation level: its presentation
 * buffers already span the complete physical output. Standard currently
 * sizes its VRAM desktop texture from desktop_width/desktop_height, so
 * Standard remains deliberately rejected here until its resize contract is
 * added before the first Standard mode becomes selectable.
 */
static int ps2vnc_reconfigure_current_desktop_hires(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    unsigned int out_x,
    unsigned int out_y,
    unsigned int width,
    unsigned int height)
{
    if (active_display_backend != PS2VNC_BACKEND_HIRES)
        return -1;

    if (!region_fits_u32(
            out_x, out_y, width, height,
            OUTPUT_WIDTH, OUTPUT_HEIGHT))
        return -1;

    /*
     * Normal live mode advertises Hextile + Raw only. Re-advertise EDS
     * before a possible SetDesktopSize transaction.
     */
    if (set_raw_extended_desktop_encoding(sock) < 0)
        return -1;

    if (desktop_width != width ||
        desktop_height != height) {

        if (request_and_confirm_desktop_size(
                sock,
                rfb_framebuffer,
                gs_framebuffer,
                width,
                height) < 0)
            return -1;
    }

    /*
     * From here forward gs_framebuffer is interpreted at the new tightly
     * packed RFB stride.
     */
    desktop_width = width;
    desktop_height = height;
    desktop_output_x = out_x;
    desktop_output_y = out_y;
    desktop_output_width = width;
    desktop_output_height = height;
    desktop_geometry_locked = 1;

    debug_emit_geometry_snapshot();

    /*
     * Calibration has temporarily replaced the visible frame. Obtain one
     * authoritative full Raw desktop and republish both HIRES buffers before
     * incremental Hextile traffic resumes.
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
            1) < 0)
        return -1;

    if (ps2vnc_display_publish_full_desktop(
            gsGlobal,
            tex,
            gs_framebuffer) < 0)
        return -1;

    if (set_live_encoding(sock) < 0)
        return -1;

    return 0;
}


/*
 * Standard keeps a fixed logical RFB desktop and VRAM texture. Runtime
 * calibration therefore changes only the physical presentation rectangle;
 * the next normal draw reuses the existing authoritative texture.
 */
static int ps2vnc_reconfigure_current_desktop_standard(
    GSTEXTURE *tex,
    unsigned int out_x,
    unsigned int out_y,
    unsigned int width,
    unsigned int height)
{
    if (active_display_backend != PS2VNC_BACKEND_STANDARD ||
        active_video_mode == NULL ||
        tex == NULL ||
        tex->Mem == NULL)
        return -1;

    /*
     * D17AL-F3:
     * Standard logical/RFB geometry is deliberately independent from its
     * GS framebuffer geometry.  In particular, 480i is 704x464 logical
     * pixels backed by a 704x232 interlaced FRAME drawing surface.
     */
    if (desktop_width != active_video_mode->standard_logical_width ||
        desktop_height != active_video_mode->standard_logical_height ||
        tex->Width != desktop_width ||
        tex->Height != desktop_height)
        return -1;

    if (!region_fits_u32(
            out_x, out_y, width, height,
            OUTPUT_WIDTH, OUTPUT_HEIGHT))
        return -1;

    desktop_output_x = out_x;
    desktop_output_y = out_y;
    desktop_output_width = width;
    desktop_output_height = height;
    desktop_geometry_locked = 1;

    debug_emit_geometry_snapshot();
    return 0;
}


/*
 * Complete live same-mode calibration transaction.
 *
 * This deliberately does NOT call ps2vnc_switch_display_mode() and does not
 * touch startup_mode. The existing calibration save path writes only the
 * active mode's [display.<mode>] safe-area values.
 *
 * D2A leaves this function unreachable; D2B will publish the semantic menu
 * request only after this transaction has built cleanly.
 */
static int ps2vnc_run_runtime_calibration(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    u64 black)
{
    unsigned int target_x;
    unsigned int target_y;
    unsigned int target_w;
    unsigned int target_h;
    int result = -1;
    int pad_owned = 0;
    int persist_calibration = 0;

    if (active_display_backend != PS2VNC_BACKEND_HIRES &&
        active_display_backend != PS2VNC_BACKEND_STANDARD)
        return -1;

    if (ps2vnc_controller_pad_acquire() < 0)
        return -1;

    pad_owned = 1;

    /*
     * The controller can no longer enqueue input after acknowledging the
     * handoff. Drain anything it queued before that acknowledgement.
     */
    if (rfb_queue_sema >= 0 &&
        flush_rfb_outgoing(sock) < 0)
        goto done;

    ps2vnc_display_direct_write_cancel();

    if (ps2vnc_display_wait_presentation_memory_idle(
            gsGlobal) < 0)
        goto done;

    if (run_screen_fit_calibration(
            gsGlobal,
            black,
            0,
            &persist_calibration) < 0)
        goto done;

    if (calculate_presented_desktop_geometry(
            &target_x,
            &target_y,
            &target_w,
            &target_h) < 0)
        goto done;

    if (active_display_backend == PS2VNC_BACKEND_HIRES) {
        if (ps2vnc_reconfigure_current_desktop_hires(
                sock,
                rfb_framebuffer,
                gs_framebuffer,
                gsGlobal,
                tex,
                target_x,
                target_y,
                target_w,
                target_h) < 0)
            goto done;
    } else {
        if (ps2vnc_reconfigure_current_desktop_standard(
                tex,
                target_x,
                target_y,
                target_w,
                target_h) < 0)
            goto done;
    }

    /*
     * Persist only after the complete live geometry transaction succeeds.
     * Save failure is non-fatal: the calibrated live session remains valid,
     * exactly like startup-mode persistence after a successful mode switch.
     *
     * Circle/FULL does not request persistence, preserving the historical
     * calibration bypass behavior.
     */
    if (persist_calibration) {
        if (ps2vnc_display_save_to_pi(
                screen_fit_width,
                screen_fit_height,
                screen_fit_offset_x,
                screen_fit_offset_y) < 0) {

            screenf("CONFIG: display save failed");

        } else {
            screenf("CONFIG: display saved");
        }
    }

    result = 0;

done:
    if (pad_owned) {
        /*
         * Keep exclusive main-thread pad ownership through physical release,
         * then return it to the controller and wait for the ack to drop.
         */
        if (ps2vnc_controller_pad_wait_release() < 0)
            result = -1;

        if (ps2vnc_controller_pad_release() < 0)
            result = -1;
    }

    return result;
}


/*
 * TEST15E1-B:
 * Construct one mode-dependent GS/HIRES display instance.
 *
 * DMAKit remains process/session lifetime for now.  Runtime mode switching
 * reconstructs only the resources whose geometry and GS video state depend
 * on active_video_mode.
 */
static GSGLOBAL *ps2vnc_display_create_hires(void)
{
    GSGLOBAL *gsGlobal;

    debug_stage_id = DBG_STAGE_GS_INIT;

    gsGlobal = gsKit_hires_init_global();
    if (gsGlobal == NULL)
        return NULL;

    gsGlobal->Mode = active_video_mode->gs_mode;
    gsGlobal->Interlace = active_video_mode->interlace;
    gsGlobal->Field = active_video_mode->field;
    gsGlobal->Width = ps2vnc_hires_effective_width();
    gsGlobal->Height = active_video_mode->hires_height;

    gsGlobal->PSM = GS_PSM_CT16S;
    gsGlobal->PSMZ = GS_PSMZ_16S;
    gsGlobal->Dithering = GS_SETTING_ON;
    gsGlobal->DoubleBuffering = GS_SETTING_ON;
    gsGlobal->ZBuffering = GS_SETTING_OFF;
    gsGlobal->PrimAlphaEnable = GS_SETTING_OFF;

    gsKit_vram_clear(gsGlobal);
    gsKit_hires_init_screen(
        gsGlobal,
        active_video_mode->hires_passes
    );

    gsKit_set_display_offset(gsGlobal, 0, 0);
    gsKit_set_clamp(gsGlobal, GS_CMODE_CLAMP);

    return gsGlobal;
}

/*
 * TEST15E3K A14-D:
 * Construct one ordinary-gsKit display from explicit Standard renderer
 * metadata. DMAKit remains process/session lifetime and is not reinitialized
 * here during display reconstruction.
 */
static GSGLOBAL *ps2vnc_display_create_standard(void)
{
    GSGLOBAL *gsGlobal;

    if (active_video_mode == NULL ||
        !ps2vnc_video_mode_supports_backend(
            active_video_mode,
            PS2VNC_BACKEND_STANDARD) ||
        active_video_mode->standard_fb_width == 0 ||
        active_video_mode->standard_fb_height == 0)
        return NULL;

    debug_stage_id = DBG_STAGE_GS_INIT;

    gsGlobal = gsKit_init_global();
    if (gsGlobal == NULL)
        return NULL;

    gsGlobal->Mode = active_video_mode->gs_mode;
    gsGlobal->Interlace = active_video_mode->interlace;
    gsGlobal->Field = active_video_mode->field;

    /*
     * These are GS framebuffer/drawing dimensions, not physical-raster or
     * calibrated RFB desktop dimensions. Interlaced FRAME modes therefore
     * store their already-halved GS framebuffer height in the mode record.
     */
    gsGlobal->Width = active_video_mode->standard_fb_width;
    gsGlobal->Height = active_video_mode->standard_fb_height;

    /*
     * Preserve the hardware-proven Test13 Standard presentation format.
     * The remote desktop remains a separate 16-bit texture.
     */
    gsGlobal->PSM = GS_PSM_CT32;
    gsGlobal->PSMZ = GS_PSMZ_32;
    gsGlobal->DoubleBuffering = GS_SETTING_ON;
    gsGlobal->ZBuffering = GS_SETTING_OFF;
    gsGlobal->PrimAlphaEnable = GS_SETTING_OFF;

    gsKit_vram_clear(gsGlobal);
    gsKit_init_screen(gsGlobal);

    gsKit_set_display_offset(
        gsGlobal,
        active_video_mode->standard_offset_x,
        active_video_mode->standard_offset_y);

    gsKit_mode_switch(gsGlobal, GS_ONESHOT);
    gsKit_set_clamp(gsGlobal, GS_CMODE_CLAMP);

    return gsGlobal;
}

/*
 * TEST15E3-K:
 * Backend-neutral display construction boundary.
 *
 * Current supported modes are HIRES-only, so this dispatcher is deliberately
 * behavior-preserving. Future Standard-gsKit support enters here without
 * teaching callers about renderer-specific initialization.
 */
static GSGLOBAL *ps2vnc_display_create(void)
{
    ps2vnc_backend_t backend =
        ps2vnc_video_mode_default_backend(active_video_mode);
    GSGLOBAL *display = NULL;

    if (backend == PS2VNC_BACKEND_HIRES)
        display = ps2vnc_display_create_hires();
    else if (backend == PS2VNC_BACKEND_STANDARD)
        display = ps2vnc_display_create_standard();

    if (display != NULL)
        active_display_backend = backend;

    return display;
}

/*
 * TEST15E1-B:
 * Matching display-instance destructor.  E1-B deliberately does not call
 * this yet; E1-C will introduce an explicit lifecycle boundary rather than
 * silently changing every existing display_live_desktop() return path.
 */
static void ps2vnc_display_release_ee_buffers(void)
{
    unsigned short *front = hires_bg_buffer;
    unsigned short *back = hires_bg_back_buffer;

    live_direct_present_safe = 0;
    live_direct_present_buffer = NULL;
    hires_mapped_source_framebuffer = NULL;
    live_linear_framebuffer_stale = 0;
    hires_bg_configured = 0;
    hires_bg_buffer = NULL;
    hires_bg_back_buffer = NULL;

    if (back != NULL && back != front)
        free(back);
    if (front != NULL)
        free(front);
}

static void ps2vnc_display_destroy_hires(GSGLOBAL **display)
{
    GSGLOBAL *gsGlobal = *display;

    gsKit_hires_sync(gsGlobal);
    dmaKit_wait_fast();
    gsKit_hires_deinit_global(gsGlobal);
    ps2vnc_display_release_ee_buffers();

    *display = NULL;
}

static void ps2vnc_display_destroy_standard(GSGLOBAL **display)
{
    GSGLOBAL *gsGlobal = *display;

    /*
     * Standard gsKit owns no PS2VNC EE presentation buffers. The normal
     * queue/flip lifecycle has already exposed completed rendering.
     */
    gsKit_deinit_global(gsGlobal);
    *display = NULL;
}

/*
 * Wait until renderer-owned EE presentation memory may safely be written.
 */
static int ps2vnc_display_wait_presentation_memory_idle(
    GSGLOBAL *gsGlobal)
{
    if (gsGlobal == NULL)
        return -1;

    if (active_display_backend == PS2VNC_BACKEND_HIRES) {
        gsKit_hires_sync(gsGlobal);
        dmaKit_wait_fast();
        return 0;
    }

    if (active_display_backend == PS2VNC_BACKEND_STANDARD) {
        /*
         * Standard decoding never writes renderer-owned EE presentation
         * memory, so there is no ownership barrier to wait for here.
         */
        return 0;
    }

    return -1;
}

/*
 * Wait until a texture consumed by the previous frame may safely be replaced.
 */
static int ps2vnc_display_wait_texture_idle(
    GSGLOBAL *gsGlobal)
{
    if (gsGlobal == NULL)
        return -1;

    if (active_display_backend == PS2VNC_BACKEND_HIRES) {
        gsKit_hires_sync(gsGlobal);
        return 0;
    }

    if (active_display_backend == PS2VNC_BACKEND_STANDARD) {
        /*
         * Proven Standard renderers upload/update textures only after the
         * preceding DRAW -> sync_flip cycle has completed.
         */
        return 0;
    }

    return -1;
}

/*
 * Complete construction of the current draw queue.
 *
 * HIRES consumes its queue through the scanline/pass engine.  A future
 * Standard backend maps this semantic operation to gsKit_queue_exec().
 */
static int ps2vnc_display_finish_draw(GSGLOBAL *gsGlobal)
{
    if (gsGlobal == NULL)
        return -1;

    if (active_display_backend == PS2VNC_BACKEND_HIRES) {
        gsKit_hires_sync(gsGlobal);
        return 0;
    }

    if (active_display_backend == PS2VNC_BACKEND_STANDARD) {
        gsKit_queue_exec(gsGlobal);
        return 0;
    }

    return -1;
}

/*
 * Expose the completed frame.
 *
 * HIRES advances its scanline-managed presentation buffers.  A future
 * Standard backend maps this semantic operation to gsKit_sync_flip().
 */
static int ps2vnc_display_flip(GSGLOBAL *gsGlobal)
{
    if (gsGlobal == NULL)
        return -1;

    if (active_display_backend == PS2VNC_BACKEND_HIRES) {
        gsKit_hires_flip(gsGlobal);
        return 0;
    }

    if (active_display_backend == PS2VNC_BACKEND_STANDARD) {
        gsKit_sync_flip(gsGlobal);
        return 0;
    }

    return -1;
}

static int ps2vnc_display_destroy(GSGLOBAL **display)
{
    if (display == NULL || *display == NULL) {
        active_display_backend = PS2VNC_BACKEND_NONE;
        return 0;
    }

    if (active_display_backend == PS2VNC_BACKEND_HIRES) {
        ps2vnc_display_destroy_hires(display);
        active_display_backend = PS2VNC_BACKEND_NONE;
        return 0;
    }

    if (active_display_backend == PS2VNC_BACKEND_STANDARD) {
        ps2vnc_display_destroy_standard(display);
        active_display_backend = PS2VNC_BACKEND_NONE;
        return 0;
    }

    return -1;
}


static int ps2vnc_display_prepare_desktop_hires(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    unsigned short *gs_framebuffer)
{
    if (hires_bg_buffer != NULL || hires_bg_back_buffer != NULL)
        return -1;

    if (ps2vnc_hires_storage_width() == 0 ||
        ps2vnc_hires_storage_height() == 0)
        return -1;

    memset(tex, 0, sizeof(*tex));
    tex->Width = ps2vnc_hires_storage_width();
    tex->Height = ps2vnc_hires_storage_height();
    tex->PSM = GS_PSM_CT16S;
    tex->TBW = ps2vnc_hires_storage_width() / 64;
    tex->Filter = GS_FILTER_NEAREST;
    tex->VramClut = 0;

    hires_bg_buffer = memalign(
        128, ps2vnc_hires_storage_bytes());
    if (hires_bg_buffer == NULL)
        return -1;

    hires_bg_back_buffer = memalign(
        128, ps2vnc_hires_storage_bytes());
    if (hires_bg_back_buffer == NULL) {
        ps2vnc_display_release_ee_buffers();
        return -1;
    }

    hires_bg_configured = 0;

    hires_mapped_source_framebuffer =
        ps2vnc_hires_requires_raster_mapping()
            ? gs_framebuffer
            : NULL;

    if (publish_hires_background(gsGlobal, tex, gs_framebuffer) < 0) {
        ps2vnc_display_release_ee_buffers();
        return -1;
    }

    if (ps2vnc_hires_requires_raster_mapping()) {
        live_direct_present_safe = 0;
        live_direct_present_buffer = NULL;
    } else {
        live_direct_present_buffer = hires_bg_back_buffer;
    }

    memcpy(
        hires_bg_back_buffer,
        hires_bg_buffer,
        ps2vnc_hires_storage_bytes());

    SyncDCache(
        hires_bg_back_buffer,
        (unsigned char *)hires_bg_back_buffer +
            ps2vnc_hires_storage_bytes());

    return 0;
}

/*
 * Ordinary gsKit presents the authoritative linear GS16 framebuffer as a
 * conventional VRAM texture. Unlike HIRES, no extra EE presentation buffers
 * or decoder-direct target exist.
 */
static int ps2vnc_display_prepare_desktop_standard(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    unsigned short *gs_framebuffer)
{
    if (gsGlobal == NULL ||
        tex == NULL ||
        gs_framebuffer == NULL ||
        desktop_width == 0 ||
        desktop_height == 0)
        return -1;

    memset(tex, 0, sizeof(*tex));

    tex->Width = desktop_width;
    tex->Height = desktop_height;
    tex->PSM = GS_PSM_CT16;
    tex->Mem = (u32 *)gs_framebuffer;
    tex->Filter = GS_FILTER_NEAREST;
    tex->VramClut = 0;

    tex->Vram = gsKit_vram_alloc(
        gsGlobal,
        gsKit_texture_size(
            tex->Width,
            tex->Height,
            tex->PSM),
        GSKIT_ALLOC_USERBUFFER);

    debug_stage_id = DBG_STAGE_INITIAL_TEXTURE;
    gsKit_texture_upload(gsGlobal, tex);

    live_linear_framebuffer_stale = 0;
    return 0;
}

/*
 * Backend-neutral desktop preparation.
 */
static int ps2vnc_display_prepare_desktop(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    unsigned short *gs_framebuffer)
{
    if (active_display_backend == PS2VNC_BACKEND_HIRES)
        return ps2vnc_display_prepare_desktop_hires(
            gsGlobal, tex, gs_framebuffer);

    if (active_display_backend == PS2VNC_BACKEND_STANDARD)
        return ps2vnc_display_prepare_desktop_standard(
            gsGlobal, tex, gs_framebuffer);

    return -1;
}

/*
 * Draw the current desktop through the active presentation backend.
 *
 * HIRES supplies the desktop through its background/pass engine, so this
 * operation is intentionally a no-op there.
 *
 * Standard owns a conventional VRAM texture. Reproduce the proven ordinary
 * gsKit frame ordering by clearing the back buffer and explicitly drawing
 * that texture before local overlays are queued.
 */
static int ps2vnc_display_draw_desktop(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    u64 clear_color,
    u64 tex_color)
{
    float x0, y0, x1, y1;

    if (gsGlobal == NULL || tex == NULL)
        return -1;

    if (active_display_backend == PS2VNC_BACKEND_HIRES)
        return 0;

    if (active_display_backend != PS2VNC_BACKEND_STANDARD)
        return -1;

    if (tex->Width != desktop_width ||
        tex->Height != desktop_height ||
        tex->Mem == NULL)
        return -1;

    x0 = ps2vnc_display_output_x((float)desktop_output_x);
    y0 = ps2vnc_display_output_y((float)desktop_output_y);

    x1 = ps2vnc_display_output_x(
        (float)(desktop_output_x + desktop_output_width));
    y1 = ps2vnc_display_output_y(
        (float)(desktop_output_y + desktop_output_height));

    gsKit_clear(gsGlobal, clear_color);

    gsKit_prim_sprite_texture(
        gsGlobal,
        tex,
        x0, y0,
        0.0f, 0.0f,
        x1, y1,
        (float)tex->Width,
        (float)tex->Height,
        1,
        tex_color);

    return 0;
}

static void ps2vnc_display_prepare_osk(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *osk_tex)
{
    memset(osk_tex, 0, sizeof(*osk_tex));
    osk_tex->Width = OSK_WIDTH;
    osk_tex->Height = OSK_HEIGHT;
    osk_tex->PSM = GS_PSM_CT16;
    osk_tex->Mem = (u32 *)osk_pixels;
    osk_tex->Filter = GS_FILTER_NEAREST;
    osk_tex->VramClut = 0;
    osk_tex->Vram = gsKit_vram_alloc(
        gsGlobal,
        gsKit_texture_size(OSK_WIDTH, OSK_HEIGHT, GS_PSM_CT16),
        GSKIT_ALLOC_USERBUFFER);
}


/*
 * TEST15E1-D1:
 * Reconstruct the complete mode-dependent display while preserving the
 * RFB connection and controller thread.
 */
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
static int ps2vnc_switch_display_mode(
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
static int ps2vnc_startup_finalize_display_transaction(void)
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


static int ps2vnc_remote_result_post(
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


static void ps2vnc_remote_display_fail(
    unsigned int error_code)
{
    remote_display_error_code = error_code;

    (void)ps2vnc_remote_result_post(
        "FAILED",
        error_code);

    ps2vnc_remote_display_reset();
}


static int ps2vnc_remote_display_lease_expired(void)
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


static int ps2vnc_remote_control_poll_idle(void)
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
            ps2vnc_video_mode_by_name(mode_name);

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


static int ps2vnc_global_rollback_display_transaction(
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
    debug_stage_id = DBG_STAGE_CONTROLLER_START;

    rfb_connected = 1;

    if (start_controller_thread(sock) < 0) {
        rfb_connected = 0;
        return -1;
    }

    for (;;) {
        if (exit_requested)
            return 1;

        debug_frame_counter++;
        debug_stage_id = DBG_STAGE_LOOP_BEGIN;
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

        debug_stage_id = DBG_STAGE_PRE_DRAW;

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

        debug_stage_id = DBG_STAGE_POST_DRAW;

        /*
         * TEST13E HIRES presentation.
         *
         * Preserve the logical DRAW -> FLIP ordering, but HIRES performs
         * the actual rendering in scanline-driven passes.
         *
         * Legacy debug stage IDs are retained intentionally.
         */
        debug_stage_id = DBG_STAGE_PRE_QUEUE;
        {
            u64 t0 = GetTimerSystemTime();

            if (ps2vnc_display_finish_draw(gsGlobal) < 0)
                return -1;

            profile_present_sync_ticks += GetTimerSystemTime() - t0;
        }
        debug_stage_id = DBG_STAGE_POST_QUEUE;

        debug_stage_id = DBG_STAGE_PRE_FLIP;
        {
            u64 t0 = GetTimerSystemTime();

            if (ps2vnc_display_flip(gsGlobal) < 0)
                return -1;

            profile_flip_ticks += GetTimerSystemTime() - t0;
        }
        debug_stage_id = DBG_STAGE_POST_FLIP;

        /*
         * After the initial complete frame, request only pixels that changed.
         * Queued controller traffic is flushed before this main-thread request.
         */
        if (controller_thread_error) {
            debug_stage_id = DBG_STAGE_CONTROLLER_ERROR;
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

        debug_stage_id = DBG_STAGE_PRE_REQUEST;
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

            debug_stage_id = DBG_STAGE_REQUEST_ERROR;
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

        debug_stage_id = DBG_STAGE_POST_REQUEST;

        {
            int changed_bytes;
            int direct_present_complete;

            debug_stage_id = DBG_STAGE_WAIT_RFB;

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
            debug_stage_id = DBG_STAGE_POST_RECEIVE;

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

                debug_stage_id = DBG_STAGE_RECEIVE_ERROR;
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
                debug_stage_id = DBG_STAGE_PRE_TEXTURE;

                if (ps2vnc_display_commit_update(
                        gsGlobal,
                        &tex,
                        direct_present_complete) < 0)
                    return -1;

                debug_stage_id = DBG_STAGE_POST_TEXTURE;
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

    debug_stage_id = DBG_STAGE_NET_READY;
    if (debug_udp_init() < 0)
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
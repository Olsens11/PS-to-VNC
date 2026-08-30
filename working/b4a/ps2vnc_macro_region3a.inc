

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

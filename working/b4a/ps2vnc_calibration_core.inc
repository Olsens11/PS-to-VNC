#include "video/geometry.h"



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


int ps2vnc_display_transaction_startup_preflight(void)
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
int ps2vnc_startup_prepare_display_transaction(void)
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
int ps2vnc_startup_connect_reconciled(
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
        pstvnc_video_mode_by_name(mode->name) == NULL)
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


int ps2vnc_display_save_to_pi(
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
int ps2vnc_config_get_display_for_mode(
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

int ps2vnc_config_presented_geometry_for_mode(
    const ps2vnc_video_mode_t *mode,
    const ps2vnc_display_config_t *config,
    unsigned int *out_x,
    unsigned int *out_y,
    unsigned int *out_w,
    unsigned int *out_h)
{
    pstvnc_video_rect_t safe_area;
    pstvnc_video_rect_t presented;

    if (mode == NULL || config == NULL ||
        out_x == NULL || out_y == NULL ||
        out_w == NULL || out_h == NULL)
        return -1;

    if (pstvnc_video_geometry_safe_area_rect(
            mode,
            config->safe_width,
            config->safe_height,
            config->safe_x,
            config->safe_y,
            &safe_area) < 0)
        return -1;

    if (pstvnc_video_geometry_presented_rect(
            mode,
            &safe_area,
            VNC_WIDTH,
            VNC_HEIGHT,
            &presented) < 0)
        return -1;

    *out_x = presented.x;
    *out_y = presented.y;
    *out_w = presented.width;
    *out_h = presented.height;

    return 0;
}



 int calculate_presented_desktop_geometry(
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
int ps2vnc_startup_apply_authoritative_profile(void)
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
        pstvnc_video_mode_default_backend(target->mode);

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



int ps2vnc_config_load_from_pi(void)
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
void debug_emit_geometry_snapshot(void)
{
    char msg[192];
    int len;

    if (!pstvnc_debug_is_ready())
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

    (void)pstvnc_debug_send(
        msg,
        (size_t)len);
}


/*
 * Interactive calibration policy.
 *
 * Size and position are constrained together against the physical
 * output coordinate space.
 */
void screen_fit_clamp_interactive(
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

void screen_fit_prim_sprite(
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

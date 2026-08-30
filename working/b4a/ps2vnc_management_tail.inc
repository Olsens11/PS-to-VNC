int ps2vnc_display_profile_valid(
    const ps2vnc_display_profile_t *profile)
{
    unsigned int expected_logical_w;
    unsigned int expected_logical_h;

    if (profile == NULL ||
        profile->mode == NULL ||
        profile->mode->name == NULL)
        return 0;

    if (!pstvnc_video_mode_supports_backend(
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

    backend = pstvnc_video_mode_default_backend(mode);

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


int ps2vnc_management_get(
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
            pstvnc_config_trim_left(line);

        pstvnc_config_trim_right(work);

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
                pstvnc_config_trim_left(work);

            pstvnc_config_trim_right(work);

            if (strcmp(work, "display") == 0) {

                current_mode = -1;

            } else if (
                strncmp(
                    work,
                    "display.",
                    8) == 0) {

                const ps2vnc_video_mode_t *mode =
                    pstvnc_video_mode_by_name(
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
            pstvnc_config_trim_left(work);

        pstvnc_config_trim_right(key);

        value =
            pstvnc_config_trim_left(
                equals + 1);

        pstvnc_config_trim_right(value);

        if (current_mode == -1 &&
            strcmp(
                key,
                "hide_locked_modes") == 0) {

            int parsed;

            if (hide_seen ||
                !pstvnc_config_parse_bool(
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
                !pstvnc_config_parse_bool(
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
int ps2vnc_display_tx_take_line(
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
        !pstvnc_config_parse_int(
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


int ps2vnc_display_tx_parse_profile(
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

    profile->mode = pstvnc_video_mode_by_name(value);

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

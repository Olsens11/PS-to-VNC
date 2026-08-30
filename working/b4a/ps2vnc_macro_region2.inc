
static int start_controller_thread(int sock)
{
    ee_sema_t sema;
    ee_thread_t thread;

    memset(&sema, 0, sizeof(sema));
    sema.init_count = 1;
    sema.max_count = 1;
    sema.option = 0;

    rfb_queue_sema = CreateSema(&sema);
    if (rfb_queue_sema < 0)
        return -1;

    rfb_out_head = 0;
    rfb_out_tail = 0;
    rfb_out_count = 0;

    controller_sock = sock;
    controller_thread_error = 0;
    exit_requested = 0;

    memset(&thread, 0, sizeof(thread));
    thread.func = (void *)controller_thread;
    thread.stack = (void *)controller_stack;
    thread.stack_size = CONTROLLER_STACK_SIZE;
    thread.gp_reg = &_gp;
    thread.initial_priority = 64;
    thread.attr = 0;
    thread.option = 0;

    controller_thread_id = CreateThread(&thread);
    if (controller_thread_id < 0)
        return -1;

    if (StartThread(controller_thread_id, (void *)&controller_sock) < 0)
        return -1;

    return 0;
}

static int request_framebuffer(int sock, int incremental)
{
    unsigned char msg[10] = {
        3, 0,
        0, 0,
        0, 0,
        (desktop_width >> 8) & 0xFF,
        desktop_width & 0xFF,
        (desktop_height >> 8) & 0xFF,
        desktop_height & 0xFF
    };

    /*
     * FramebufferUpdateRequest byte 1:
     *   0 = non-incremental: send the complete requested region
     *   1 = incremental: send only regions changed since our last request
     */
    msg[1] = incremental ? 1 : 0;

    if (rfb_queue_sema >= 0) {
        if (flush_rfb_outgoing(sock) < 0)
            return -1;
    }

    return send_exact(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}

/*
 * RFB is requested directly as GS-compatible B5:G5:R5.
 * Bit 15 is unused on the wire; set the GS A1 bit locally.
 */
static unsigned short rfb_gs555_to_gs16(unsigned short p)
{
    return (unsigned short)(p | 0x8000);
}

static void hextile_store_pixel(
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    unsigned int index,
    unsigned short *direct_pixel,
    unsigned short wire_pixel)
{
    unsigned short gs_pixel = rfb_gs555_to_gs16(wire_pixel);

    if (direct_pixel != NULL) {
        *direct_pixel = gs_pixel;
        return;
    }

    if (rfb_framebuffer != gs_framebuffer)
        rfb_framebuffer[index] = wire_pixel;

    gs_framebuffer[index] = gs_pixel;
}

static int ps2vnc_display_direct_write_active(void);
static unsigned short *ps2vnc_display_direct_write_row(
    unsigned int x,
    unsigned int y);
static void ps2vnc_display_direct_write_cancel(void);
static int ps2vnc_display_begin_update(void);
static int ps2vnc_display_finish_update_decode(void);


static void hextile_fill_rect(
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    unsigned int x,
    unsigned int y,
    unsigned int w,
    unsigned int h,
    unsigned short wire_pixel)
{
    unsigned int row, col;

    for (row = 0; row < h; row++) {
        unsigned int base = (y + row) * desktop_width + x;
        unsigned short *direct_row = NULL;

        direct_row = ps2vnc_display_direct_write_row(
            x,
            y + row);

        for (col = 0; col < w; col++)
            hextile_store_pixel(
                rfb_framebuffer, gs_framebuffer, base + col,
                direct_row ? &direct_row[col] : NULL,
                wire_pixel);
    }
}


static int hextile_recv_pixel(
    int sock, unsigned short *pixel, unsigned int *wire_bytes)
{
    if (recv_exact(sock, pixel, 2) != 2)
        return -1;

    *wire_bytes += 2;
    return 0;
}

static int hextile_recv_raw_tile(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    unsigned int tx, unsigned int ty,
    unsigned int tw, unsigned int th,
    unsigned int *wire_bytes,
    int write_pixels)
{
    unsigned short scratch[16];
    unsigned int row, col;

    for (row = 0; row < th; row++) {
        unsigned short *dest =
            (write_pixels && !ps2vnc_display_direct_write_active())
            ? &rfb_framebuffer[(ty + row) * desktop_width + tx]
            : scratch;
        unsigned int n = tw * 2;

        if (recv_exact(sock, dest, n) != (int)n)
            return -1;

        *wire_bytes += n;

        if (write_pixels) {
            unsigned int base =
                (ty + row) * desktop_width + tx;
            unsigned short *direct_row = NULL;

            direct_row = ps2vnc_display_direct_write_row(
                tx,
                ty + row);

            for (col = 0; col < tw; col++)
                hextile_store_pixel(
                    rfb_framebuffer, gs_framebuffer, base + col,
                    direct_row ? &direct_row[col] : NULL,
                    dest[col]);
        }
    }

    return 0;
}


static int hextile_recv_subrects(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    unsigned int tx, unsigned int ty,
    unsigned int tw, unsigned int th,
    unsigned short foreground,
    int colored,
    unsigned int count,
    unsigned int *wire_bytes,
    int write_pixels)
{
    unsigned int i;

    for (i = 0; i < count; i++) {
        unsigned short pixel = foreground;
        unsigned char geom[2];
        unsigned int sx, sy, sw, sh;

        if (colored &&
            hextile_recv_pixel(sock, &pixel, wire_bytes) < 0)
            return -1;

        if (recv_exact(sock, geom, 2) != 2)
            return -1;

        *wire_bytes += 2;

        sx = geom[0] >> 4;
        sy = geom[0] & 0x0f;
        sw = (geom[1] >> 4) + 1;
        sh = (geom[1] & 0x0f) + 1;

        if (sx + sw > tw || sy + sh > th)
            return -1;

        if (write_pixels)
            hextile_fill_rect(
                rfb_framebuffer, gs_framebuffer,
                tx + sx, ty + sy, sw, sh, pixel);
    }

    return 0;
}


static int hextile_recv_rect(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    unsigned int x, unsigned int y,
    unsigned int w, unsigned int h,
    unsigned int *wire_bytes,
    int write_pixels)
{
    unsigned short bg = 0, fg = 0;
    int bg_valid = 0, fg_valid = 0;
    unsigned int ty, tx;

    for (ty = y; ty < y + h; ty += 16) {
        unsigned int th = ((y + h) - ty > 16) ? 16 : (y + h) - ty;

        for (tx = x; tx < x + w; tx += 16) {
            unsigned int tw = ((x + w) - tx > 16) ? 16 : (x + w) - tx;
            unsigned char sub;

            if (recv_exact(sock, &sub, 1) != 1)
                return -1;
            (*wire_bytes)++;

            if (sub & ~0x1fu)
                return -1;

            if (sub & HEXTILE_RAW) {
                if (hextile_recv_raw_tile(
                        sock, rfb_framebuffer, gs_framebuffer,
                        tx, ty, tw, th, wire_bytes,
                        write_pixels) < 0)
                    return -1;

                bg_valid = 0;
                fg_valid = 0;
                continue;
            }

            if (sub & HEXTILE_BACKGROUND_SPECIFIED) {
                if (hextile_recv_pixel(sock, &bg, wire_bytes) < 0)
                    return -1;
                bg_valid = 1;
            }

            if (!bg_valid)
                return -1;

            if (write_pixels)
                hextile_fill_rect(
                    rfb_framebuffer, gs_framebuffer,
                    tx, ty, tw, th, bg);

            /*
             * RFC 6143: ForegroundSpecified and SubrectsColored
             * may not both be set for the same tile.
             */
            if ((sub & HEXTILE_FOREGROUND_SPECIFIED) &&
                (sub & HEXTILE_SUBRECTS_COLORED))
                return -1;

            if (sub & HEXTILE_FOREGROUND_SPECIFIED) {
                if (hextile_recv_pixel(sock, &fg, wire_bytes) < 0)
                    return -1;
                fg_valid = 1;
            }

            if (sub & HEXTILE_ANY_SUBRECTS) {
                unsigned char count;
                int colored =
                    (sub & HEXTILE_SUBRECTS_COLORED) != 0;

                if (recv_exact(sock, &count, 1) != 1)
                    return -1;
                (*wire_bytes)++;

                if (!colored && !fg_valid)
                    return -1;

                if (hextile_recv_subrects(
                        sock, rfb_framebuffer, gs_framebuffer,
                        tx, ty, tw, th, fg, colored,
                        count, wire_bytes, write_pixels) < 0)
                    return -1;

            }

            /*
             * A tile carrying SubrectsColored invalidates foreground
             * carry for the following tile, even if it had no subrects.
             */
            if (sub & HEXTILE_SUBRECTS_COLORED)
                fg_valid = 0;
        }
    }

    return 0;
}

/*
 * Receive one Raw FramebufferUpdate.
 *
 * TEST15B2E:
 * RFB pixels arrive in GS-compatible B5:G5:R5 ordering. rfb_framebuffer
 * may alias gs_framebuffer, allowing each Raw row to be received directly
 * into the authoritative framebuffer and finalized in place by setting A1.
 * Incremental rectangles modify only the pixels supplied by the server.
 *
 * Returns:
 *   >0  number of raw pixel bytes received
 *    0  valid update containing zero rectangles
 *   -1  protocol / socket error
 */
static int discard_exact(int sock, unsigned int len)
{
    unsigned char discard[256];

    while (len > 0) {
        unsigned int chunk = len;

        if (chunk > sizeof(discard))
            chunk = sizeof(discard);

        if (recv_exact(sock, discard, (int)chunk) != (int)chunk)
            return -1;

        len -= chunk;
    }

    return 0;
}

/*
 * Receive the next FramebufferUpdate, while safely consuming other legal
 * asynchronous RFB server messages that may arrive first.
 *
 * Important example:
 *   Dragging across text in xterm changes the X selection/clipboard.
 *   TigerVNC may then send ServerCutText (server message type 3).
 *
 * Earlier milestones assumed the first byte received here was always type 0
 * (FramebufferUpdate), so selecting text looked like a VNC/video freeze.
 *
 * Supported server message types:
 *   0 = FramebufferUpdate
 *   1 = SetColorMapEntries   (consumed/ignored; we use true-color)
 *   2 = Bell                 (no payload; ignored)
 *   3 = ServerCutText        (clipboard payload consumed/ignored)
 *
 * Returns:
 *   >0  number of raw framebuffer pixel bytes received
 *    0  valid FramebufferUpdate containing zero rectangles
 *   -1  protocol / socket error
 *   -2  internal local-redraw yield before the next server-message byte
 */
static int receive_framebuffer_update(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    int verbose,
    int require_full)
{
    for (;;) {
        unsigned char message_type;

        if (verbose)
            screenf("Waiting for server message...");

        {
            int message_result;

            rfb_message_boundary_local_yield = 1;

            message_result =
                recv_exact(sock, &message_type, 1);

            rfb_message_boundary_local_yield = 0;

            if (message_result == RFB_RECEIVE_LOCAL_REDRAW)
                return RFB_RECEIVE_LOCAL_REDRAW;

            if (message_result != 1) {
                live_error_code = 101;

                if (verbose)
                    screenf(
                        "ERROR: failed reading server message type"
                    );

                return -1;
            }
        }

        live_message_type = message_type;

        if (message_type == 2) {
            /* Bell has no payload. */
            if (verbose)
                screenf("Ignoring RFB Bell");
            continue;
        }

        if (message_type == 3) {
            unsigned char cut_hdr[7];
            unsigned int text_len;

            /*
             * ServerCutText:
             *   1 byte type (already read)
             *   3 bytes padding
             *   4 bytes text length, big-endian
             *   N bytes text
             */
            if (recv_exact(sock, cut_hdr, sizeof(cut_hdr)) !=
                sizeof(cut_hdr)) {
                live_error_code = 103;
                if (verbose)
                    screenf("ERROR: failed reading ServerCutText header");
                return -1;
            }

            text_len = read_be32(&cut_hdr[3]);

            if (verbose)
                screenf("Ignoring ServerCutText (%u bytes)", text_len);

            if (discard_exact(sock, text_len) < 0) {
                live_error_code = 104;
                if (verbose)
                    screenf("ERROR: failed discarding ServerCutText");
                return -1;
            }

            continue;
        }

        if (message_type == 1) {
            unsigned char cmap_hdr[5];
            unsigned int color_count;
            unsigned int payload_len;

            /*
             * SetColorMapEntries:
             *   1 byte type (already read)
             *   1 byte padding
             *   2 bytes first-color
             *   2 bytes color count
             *   count * 6 bytes RGB data
             *
             * We explicitly request true-color RGB565, so this should not be
             * needed, but consuming it keeps the RFB stream synchronized.
             */
            if (recv_exact(sock, cmap_hdr, sizeof(cmap_hdr)) !=
                sizeof(cmap_hdr)) {
                live_error_code = 105;
                if (verbose)
                    screenf("ERROR: failed reading color-map header");
                return -1;
            }

            color_count = read_be16(&cmap_hdr[3]);
            payload_len = color_count * 6;

            if (verbose)
                screenf("Ignoring color-map update (%u entries)",
                        color_count);

            if (discard_exact(sock, payload_len) < 0) {
                live_error_code = 106;
                if (verbose)
                    screenf("ERROR: failed discarding color map");
                return -1;
            }

            continue;
        }

        if (message_type == 0) {
            unsigned char update_tail[3];
            unsigned int rect_count;
            unsigned int rect_index;
            unsigned int total_bytes = 0;

            /*
             * TEST13F:
             * Build one bounding box covering every rectangle in this
             * completed FramebufferUpdate.
             */
            rfb_dirty_reset();

            /*
             * FramebufferUpdate after the already-consumed type byte:
             *   1 byte padding
             *   2 bytes rectangle count
             *
             * Test32 records these exact bytes because Test31b corrupted
             * the rectangle count before the first bogus rectangle header.
             */
            live_update_raw[0] = message_type;
            live_update_start_pos = rfb_rx_pos;
            live_update_start_end = rfb_rx_end;

            if (recv_exact(sock, update_tail, sizeof(update_tail)) !=
                sizeof(update_tail)) {
                live_error_code = 107;
                if (verbose)
                    screenf("ERROR: failed reading update header");
                return -1;
            }

            live_update_raw[1] = update_tail[0];
            live_update_raw[2] = update_tail[1];
            live_update_raw[3] = update_tail[2];
            live_update_end_pos = rfb_rx_pos;
            live_update_end_end = rfb_rx_end;
            live_update_recv_call = rfb_last_recv_call;

            rect_count = read_be16(&update_tail[1]);
            live_rect_count = rect_count;

            if (verbose)
                screenf("FramebufferUpdate: %u rectangle(s)", rect_count);

            if (rect_count == 0) {
                if (require_full) {
                    live_error_code = 108;
                    if (verbose)
                        screenf("ERROR: initial update had no rectangles");
                    return -1;
                }
                return 0;
            }

            for (rect_index = 0;
                 rect_index < rect_count;
                 rect_index++) {

                unsigned char rect_hdr[12];
                unsigned int x, y, w, h;
                unsigned int encoding;
                unsigned int row;
                unsigned int i;

                memset(
                    rect_hdr,
                    TEST31_HDR_SENTINEL,
                    sizeof(rect_hdr)
                );

                live_hdr_start_pos = rfb_rx_pos;
                live_hdr_start_end = rfb_rx_end;
                live_hdr_start_avail = rfb_rx_end - rfb_rx_pos;

                if (recv_exact(sock, rect_hdr, sizeof(rect_hdr)) !=
                    sizeof(rect_hdr)) {
                    live_error_code = 109;
                    live_rect_index = rect_index;
                    if (verbose)
                        screenf("ERROR: failed reading rectangle header");
                    return -1;
                }

                live_hdr_end_pos = rfb_rx_pos;
                live_hdr_end_end = rfb_rx_end;
                live_hdr_recv_n = rfb_last_recv_n;
                live_hdr_recv_call = rfb_last_recv_call;

                for (i = 0; i < sizeof(rect_hdr); i++)
                    live_hdr_raw[i] = rect_hdr[i];

                x = read_be16(&rect_hdr[0]);
                y = read_be16(&rect_hdr[2]);
                w = read_be16(&rect_hdr[4]);
                h = read_be16(&rect_hdr[6]);
                encoding = read_be32(&rect_hdr[8]);

                live_rect_index = rect_index + 1;
                live_rect_x = x;
                live_rect_y = y;
                live_rect_w = w;
                live_rect_h = h;
                live_encoding = encoding;

                if (verbose)
                    screenf("Rect %u: %u,%u %ux%u enc=%u",
                            rect_index + 1, x, y, w, h, encoding);

                /*
                 * TEST13J-D1:
                 * ExtendedDesktopSize pseudo-rectangle (-308).
                 *
                 * x = reason
                 * y = result/status
                 * w/h = framebuffer dimensions
                 *
                 * Rectangle header is followed by:
                 *   U8 screen count
                 *   3 bytes padding
                 *   count * 16-byte SCREEN structures
                 */
                if (encoding ==
                    RFB_ENCODING_EXTENDED_DESKTOP_SIZE) {

                    unsigned char layout_hdr[4];
                    unsigned int screen_count;
                    unsigned int screen_index;

                    if (recv_exact(
                            sock,
                            layout_hdr,
                            sizeof(layout_hdr)) !=
                        sizeof(layout_hdr)) {

                        live_error_code = 116;

                        if (verbose)
                            screenf(
                                "ERROR: ExtendedDesktopSize header"
                            );

                        return -1;
                    }

                    screen_count = layout_hdr[0];

                    rfb_extended_desktop_seen = 1;
                    rfb_extended_desktop_reason = x;
                    rfb_extended_desktop_result = y;
                    rfb_extended_desktop_width = w;
                    rfb_extended_desktop_height = h;
                    rfb_extended_desktop_screen_count =
                        screen_count;

                    /*
                     * Clear the first-screen snapshot before parsing
                     * the new layout.
                     */
                    rfb_first_screen_id = 0;
                    rfb_first_screen_x = 0;
                    rfb_first_screen_y = 0;
                    rfb_first_screen_width = 0;
                    rfb_first_screen_height = 0;
                    rfb_first_screen_flags = 0;

                    for (screen_index = 0;
                         screen_index < screen_count;
                         screen_index++) {

                        unsigned char screen[16];

                        if (recv_exact(
                                sock,
                                screen,
                                sizeof(screen)) !=
                            sizeof(screen)) {

                            live_error_code = 117;

                            if (verbose)
                                screenf(
                                    "ERROR: ExtendedDesktopSize screen"
                                );

                            return -1;
                        }

                        if (screen_index == 0) {
                            rfb_first_screen_id =
                                read_be32(&screen[0]);

                            rfb_first_screen_x =
                                read_be16(&screen[4]);

                            rfb_first_screen_y =
                                read_be16(&screen[6]);

                            rfb_first_screen_width =
                                read_be16(&screen[8]);

                            rfb_first_screen_height =
                                read_be16(&screen[10]);

                            rfb_first_screen_flags =
                                read_be32(&screen[12]);
                        }
                    }

                    if (verbose) {
                        screenf(
                            "ExtendedDesktopSize %ux%u screens=%u",
                            w,
                            h,
                            screen_count
                        );

                        screenf(
                            "EDS reason=%u result=%u",
                            x,
                            y
                        );

                        if (screen_count > 0) {
                            screenf(
                                "Screen0 id=%u %u,%u %ux%u",
                                rfb_first_screen_id,
                                rfb_first_screen_x,
                                rfb_first_screen_y,
                                rfb_first_screen_width,
                                rfb_first_screen_height
                            );
                        }
                    }

                    /*
                     * TEST13J-D3-A3:
                     *
                     * While a SetDesktopSize transaction is pending, the
                     * EDS rectangle must match the requested new geometry.
                     *
                     * Otherwise it describes the server's CURRENT desktop,
                     * which may already be the calibrated size left by a
                     * previous PS2VNC run.
                     */
                    if (rfb_resize_expected) {
                        if (w != rfb_resize_expected_width ||
                            h != rfb_resize_expected_height) {

                            live_error_code = 118;

                            if (verbose)
                                screenf(
                                    "ERROR: wrong resize response"
                                );

                            return -1;
                        }
                    } else {
                        if (w != desktop_width ||
                            h != desktop_height) {

                            live_error_code = 118;

                            if (verbose)
                                screenf(
                                    "ERROR: unexpected desktop resize"
                                );

                            return -1;
                        }
                    }

                    /*
                     * Pseudo-rectangle contains no Raw pixel bytes.
                     */
                    continue;
                }

                if (encoding != RFB_ENCODING_RAW &&
                    encoding != RFB_ENCODING_HEXTILE) {
                    live_error_code = 110;
                    if (verbose)
                        screenf("ERROR: unsupported encoding");
                    return -1;
                }

                if (rfb_connected) {
                    if (encoding == RFB_ENCODING_RAW)
                        profile_raw_rects++;
                    else
                        profile_hextile_rects++;
                }

                if (!region_fits_u32(
                    x, y, w, h,
                    desktop_width,
                    desktop_height)) {
                    live_error_code = 111;
                    if (verbose)
                        screenf("ERROR: rectangle outside framebuffer");
                    return -1;
                }

                /*
                 * TEST13F:
                 * Track the union of all changed rectangles. The server
                 * may send several rectangles in one FramebufferUpdate.
                 */
                if (w > 0 && h > 0)
                    rfb_dirty_mark_rect(x, y, w, h);

                if (encoding == RFB_ENCODING_HEXTILE) {
                    unsigned int hextile_bytes = 0;
                    u64 hextile_start_tick;
                    int hextile_result;
                    int write_pixels =
                        !(TEST29_DISCARD_LIVE_PIXELS && !require_full);

                    if (require_full) {
                        live_error_code = 119;
                        return -1;
                    }

                    if (write_pixels &&
                        ps2vnc_display_direct_write_active())
                        live_linear_framebuffer_stale = 1;

                    profile_hextile_active = 1;
                    hextile_start_tick = GetTimerSystemTime();
                    hextile_result = hextile_recv_rect(
                        sock, rfb_framebuffer, gs_framebuffer,
                        x, y, w, h, &hextile_bytes, write_pixels);
                    profile_hextile_ticks +=
                        GetTimerSystemTime() - hextile_start_tick;
                    profile_hextile_active = 0;

                    if (hextile_result < 0) {
                        live_error_code = 120;
                        if (verbose)
                            screenf("ERROR: malformed Hextile rectangle");
                        return -1;
                    }

                    total_bytes += hextile_bytes;
                    continue;
                }

                if (!require_full &&
                    live_linear_framebuffer_stale) {
                    ps2vnc_display_direct_write_cancel();
                    live_error_code = 121;
                    return -1;
                }

                ps2vnc_display_direct_write_cancel();

                for (row = 0; row < h; row++) {
                    unsigned int row_bytes = w * 2;

                    if (TEST29_DISCARD_LIVE_PIXELS && !require_full) {
                        /*
                         * Live isolation path: consume the exact RFB payload
                         * but do not touch either framebuffer.
                         */
                        if (row_bytes > sizeof(test29_row_scratch)) {
                            live_error_code = 115;
                            return -1;
                        }

                        if (recv_exact(
                                sock,
                                test29_row_scratch,
                                row_bytes) != (int)row_bytes) {
                            live_error_code = 112;
                            if (verbose)
                                screenf("ERROR: pixel data ended early");
                            return -1;
                        }
                    } else {
                        unsigned short *rfb_dest =
                            &rfb_framebuffer[
                                (y + row) * desktop_width + x
                            ];
                        unsigned short *gs_dest =
                            &gs_framebuffer[
                                (y + row) * desktop_width + x
                            ];
                        unsigned int col;

                        if (recv_exact(
                                sock,
                                rfb_dest,
                                row_bytes) != (int)row_bytes) {
                            live_error_code = 112;
                            if (verbose)
                                screenf("ERROR: pixel data ended early");
                            return -1;
                        }

                        for (col = 0; col < w; col++)
                            gs_dest[col] =
                                rfb_gs555_to_gs16(rfb_dest[col]);
                    }

                    total_bytes += row_bytes;
                }
            }

            if (verbose)
                screenf("Raw framebuffer bytes: %u", total_bytes);

            if (require_full &&
                total_bytes != (desktop_width * desktop_height * 2)) {
                if (verbose)
                    screenf(
                        "ERROR: initial full update expected %u bytes",
                        (unsigned int)(
                            desktop_width * desktop_height * 2
                        )
                    );
                live_error_code = 113;
                return -1;
            }

            return (int)total_bytes;
        }

        /*
         * Unknown server message type.  We cannot safely guess its payload
         * length, so stop rather than desynchronizing the RFB stream.
         */
        live_error_code = 114;
        if (verbose)
            screenf("ERROR: unsupported server message type %u",
                    (unsigned int)message_type);

        return -1;
    }
}



/*
 * TEST13J-D1:
 * Probe ExtendedDesktopSize only AFTER the ordinary complete Raw
 * framebuffer has already been received.
 *
 * The RFB specification requires a supporting server to answer a
 * non-incremental FramebufferUpdateRequest with an
 * ExtendedDesktopSize rectangle once -308 has been advertised.
 *
 * Do NOT immediately send another non-incremental request after this
 * response; that would solicit another ExtendedDesktopSize response.
 */
static int probe_extended_desktop_size(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer)
{
    rfb_extended_desktop_seen = 0;
    rfb_extended_desktop_reason = 0;
    rfb_extended_desktop_result = 0;
    rfb_extended_desktop_width = 0;
    rfb_extended_desktop_height = 0;
    rfb_extended_desktop_screen_count = 0;

    rfb_first_screen_id = 0;
    rfb_first_screen_x = 0;
    rfb_first_screen_y = 0;
    rfb_first_screen_width = 0;
    rfb_first_screen_height = 0;
    rfb_first_screen_flags = 0;

    if (set_raw_extended_desktop_encoding(sock) < 0) {
        screenf("ERROR: EDS SetEncodings failed");
        return -1;
    }

    if (request_framebuffer(sock, 0) < 0) {
        screenf("ERROR: EDS probe request failed");
        return -1;
    }

    if (receive_framebuffer_update(
            sock,
            rfb_framebuffer,
            gs_framebuffer,
            1,
            0) < 0) {

        screenf("ERROR: EDS probe receive failed");
        return -1;
    }

    if (!rfb_extended_desktop_seen) {
        screenf("ERROR: ExtendedDesktopSize unsupported");
        return -1;
    }

    if (rfb_extended_desktop_screen_count == 0) {
        screenf("ERROR: EDS reported no screens");
        return -1;
    }

    if (rfb_extended_desktop_width != desktop_width ||
        rfb_extended_desktop_height != desktop_height) {

        screenf(
            "ERROR: EDS size %ux%u",
            rfb_extended_desktop_width,
            rfb_extended_desktop_height
        );

        return -1;
    }

    screenf(
        "EDS OK %ux%u screen id=%u",
        rfb_extended_desktop_width,
        rfb_extended_desktop_height,
        rfb_first_screen_id
    );

    return 0;
}



/*
 * TEST12B:
 * Draw a visible recovery message using the same bitmap font/texture
 * machinery as the OSK. This remains visible while network calls are
 * being retried.
 */

/*
 * TEST13E-C / TEST13F:
 * HIRES pass queues reference one fixed EE-memory desktop buffer.
 * The address never changes after gsKit_hires_set_bg() configures it.
 */
static int hires_bg_is_field_separated(void)
{
    return active_video_mode->interlace == GS_INTERLACED &&
           active_video_mode->field == GS_FRAME;
}

static unsigned int hires_bg_storage_row(unsigned int raster_y)
{
    if (!hires_bg_is_field_separated())
        return raster_y;

    return (raster_y >> 1) +
           ((raster_y & 1) ? (OUTPUT_HEIGHT / 2) : 0);
}

/*
 * D17AL-F8G11B:
 *
 * A mapped destination Y coordinate is already in presentation-storage
 * space when true vertical scaling is active.
 *
 * When only horizontal mapping is active, however, Y remains physical
 * full-frame raster space. Preserve the existing even/odd field-separated
 * backing layout for interlaced GS_FRAME modes.
 */
static unsigned int ps2vnc_hires_mapped_storage_row(
    unsigned int presentation_y)
{
    if (ps2vnc_hires_requires_vertical_mapping())
        return presentation_y;

    return hires_bg_storage_row(presentation_y);
}


/*
 * Decoder-facing direct-update facade.
 *
 * RFB/Hextile knows only whether the active presentation backend offers a
 * writable hidden target and, if so, asks for a raster row.  HIRES storage
 * layout remains entirely on the presentation side of this boundary.
 */
static int ps2vnc_display_direct_write_active(void)
{
    /*
     * D17AL-F8E:
     *
     * A raster-mapped HIRES presentation cannot expose its transformed
     * presentation storage as a 1:1 decoder-direct target. Decode into the
     * authoritative source framebuffer and publish through the mapper instead.
     */
    return active_display_backend == PS2VNC_BACKEND_HIRES &&
           !ps2vnc_hires_requires_raster_mapping() &&
           live_direct_present_safe &&
           live_direct_present_buffer != NULL;
}

static unsigned short *ps2vnc_display_direct_write_row(
    unsigned int x,
    unsigned int y)
{
    if (!ps2vnc_display_direct_write_active())
        return NULL;

    if (x >= desktop_width || y >= desktop_height)
        return NULL;

    return &live_direct_present_buffer[
        hires_bg_storage_row(desktop_output_y + y) *
        OUTPUT_WIDTH + desktop_output_x + x];
}

static void ps2vnc_display_direct_write_cancel(void)
{
    live_direct_present_safe = 0;
}

static int ps2vnc_display_begin_update(void)
{
    ps2vnc_display_direct_write_cancel();

    if (active_display_backend == PS2VNC_BACKEND_HIRES &&
        !ps2vnc_hires_requires_raster_mapping())
        live_direct_present_safe = 1;

    return 0;
}

static int ps2vnc_display_finish_update_decode(void)
{
    int direct_complete = live_direct_present_safe;

    ps2vnc_display_direct_write_cancel();
    return direct_complete;
}


/*
 * TEST15D3-A3:
 * Synchronize a presentation-buffer region, optionally copying the same
 * already-complete pixels from another presentation buffer first.
 */
static int sync_copy_hires_region(
    unsigned short *dst,
    const unsigned short *src,
    unsigned int x,
    unsigned int y,
    unsigned int w,
    unsigned int h)
{
    unsigned int row;
    unsigned int row_bytes;

    if (dst == NULL || w == 0 || h == 0 ||
        !region_fits_u32(x, y, w, h, desktop_width, desktop_height))
        return -1;

    row_bytes = w * sizeof(unsigned short);

    for (row = 0; row < h; row++) {
        unsigned int i =
            hires_bg_storage_row(desktop_output_y + y + row) *
            OUTPUT_WIDTH + desktop_output_x + x;

        if (src != NULL)
            memcpy(&dst[i], &src[i], row_bytes);

        SyncDCache(&dst[i], (unsigned char *)&dst[i] + row_bytes);
    }

    return 0;
}

/*
 * TEST15D3-A3-A3:
 * Synchronize only the per-row spans touched by the completed incremental
 * FramebufferUpdate.
 */
static int sync_copy_hires_dirty_rows(
    unsigned short *dst,
    const unsigned short *src)
{
    unsigned int y;
    int saw_row = 0;

    if (dst == NULL ||
        !rfb_dirty_valid ||
        desktop_height > VNC_HEIGHT ||
        rfb_dirty_y1 > desktop_height)
        return -1;

    for (y = rfb_dirty_y0; y < rfb_dirty_y1; y++) {
        unsigned int x0;
        unsigned int x1;
        unsigned int i;
        unsigned int row_bytes;

        if (!rfb_dirty_row_valid[y])
            continue;

        x0 = rfb_dirty_row_x0[y];
        x1 = rfb_dirty_row_x1[y];

        if (x1 <= x0 || x1 > desktop_width)
            return -1;

        i =
            hires_bg_storage_row(desktop_output_y + y) *
            OUTPUT_WIDTH + desktop_output_x + x0;

        row_bytes = (x1 - x0) * sizeof(unsigned short);

        if (src != NULL)
            memcpy(&dst[i], &src[i], row_bytes);

        SyncDCache(&dst[i], (unsigned char *)&dst[i] + row_bytes);
        saw_row = 1;
    }

    return saw_row ? 0 : -1;
}

/*
 * Publish one successfully decoded incremental update.
 *
 * The generic session owns the RFB transaction. The presentation backend
 * owns how completed pixels become visible.
 */
static int publish_hires_background(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *bg_tex,
    const unsigned short *gs_framebuffer);


/*
 * D17AL-F7C1 / F8E:
 *
 * Rebuild a raster-mapped HIRES frame into the HIDDEN EE presentation buffer.
 *
 * F7B proved this ownership requirement with the hardware-validated
 * 720x480 physical/RFB -> 704x462 gsHires mapping. Rebuilding the displayed
 * buffer in place allowed the pass engine to observe black-clear ->
 * reconstructed-desktop intermediate states and produced visible flashing.
 *
 * Keep the currently displayed front completely untouched while the complete
 * replacement frame is generated. Only after the hidden frame is finished and
 * cache-visible do we point gsHires at it and swap front/back ownership.
 *
 * Mapping arithmetic remains inside publish_hires_background(); this helper
 * owns presentation-buffer atomicity rather than any particular display mode.
 */
static int ps2vnc_publish_mapped_hires_hidden_frame(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    const unsigned short *gs_framebuffer)
{
    unsigned short *old_front;
    unsigned short *hidden;

    if (!ps2vnc_hires_requires_raster_mapping() ||
        gsGlobal == NULL ||
        tex == NULL ||
        gs_framebuffer == NULL ||
        hires_bg_buffer == NULL ||
        hires_bg_back_buffer == NULL ||
        !hires_bg_configured) {

        return -1;
    }

    old_front = hires_bg_buffer;
    hidden = hires_bg_back_buffer;

    if (hidden == old_front)
        return -1;

    /*
     * publish_hires_background() owns the proven F7B conversion and writes
     * through hires_bg_buffer. Temporarily designate the hidden allocation as
     * its destination. Because hires_bg_configured is already true, this call
     * does NOT re-point gsHires while reconstruction is in progress.
     */
    hires_bg_buffer = hidden;

    if (publish_hires_background(
            gsGlobal,
            tex,
            gs_framebuffer) < 0) {

        hires_bg_buffer = old_front;
        return -1;
    }

    /*
     * Restore software front ownership until the replacement frame is fully
     * complete. Then atomically change the gsHires background contract.
     */
    hires_bg_buffer = old_front;

    tex->Mem = (u32 *)hidden;

    if (gsKit_hires_set_bg(gsGlobal, tex) < 0) {
        tex->Mem = (u32 *)old_front;
        return -1;
    }

    hires_bg_buffer = hidden;
    hires_bg_back_buffer = old_front;

    /*
     * Raster-mapped HIRES presentation storage is never exposed directly to
     * the RFB decoder.
     */
    live_direct_present_safe = 0;
    live_direct_present_buffer = NULL;

    return 0;
}

/*
 * D17AL-F7C2 / F8E:
 *
 * Update only mapped presentation pixels whose sampled authoritative
 * RFB/source pixels were touched by the completed incremental
 * FramebufferUpdate.
 *
 * F7B established the arithmetic with the hardware-validated relationship:
 *
 *     physical/calibration raster: 720x480
 *     gsHires presentation:        704x462
 *
 * Production mapping now applies the same relationship generically using the
 * active raster and presentation dimensions. rfb_dirty_row_* remains expressed
 * in authoritative RFB desktop coordinates. For each destination row that
 * samples a dirty source row, map that row's dirty x-span into the destination
 * rectangle and resample only that span.
 *
 * The leading destination edge uses ceil(source * dst / src). This is the
 * exact inverse boundary for F7B's floor(dst * src / dst) nearest-neighbor
 * sampling rule.
 */
static int ps2vnc_update_mapped_hires_dirty_buffer(
    unsigned short *dst,
    const unsigned short *gs_framebuffer)
{
    /*
     * D17AL-F7C4:
     *
     * The active raster/presentation relationship changes only when display,
     * desktop geometry, or calibration changes. Cache both forward nearest-
     * neighbor sampling and inverse dirty-boundary mappings instead of doing
     * 64-bit multiply/divide for every presentation pixel on every update.
     *
     * Normal incremental updates therefore perform table lookups only:
     *
     *     destination x/y -> source x/y
     *     source dirty edge -> destination dirty edge
     *
     * Division remains exclusively in the rare geometry-cache rebuild path.
     */

    static int map_valid = 0;

    static unsigned int cached_storage_width;
    static unsigned int cached_storage_height;

    static unsigned int cached_output_width;
    static unsigned int cached_output_height;

    static unsigned int cached_desktop_width;
    static unsigned int cached_desktop_height;

    static unsigned int cached_output_x;
    static unsigned int cached_output_y;
    static unsigned int cached_output_w;
    static unsigned int cached_output_h;

    static unsigned int cached_dst_x0;
    static unsigned int cached_dst_y0;
    static unsigned int cached_dst_x1;
    static unsigned int cached_dst_y1;
    static unsigned int cached_dst_w;
    static unsigned int cached_dst_h;

    static unsigned int x_sample[VNC_WIDTH];
    static unsigned int y_sample[VNC_HEIGHT];

    static unsigned int x_edge[VNC_WIDTH + 1];
    static unsigned int y_edge[VNC_HEIGHT + 1];

    unsigned int storage_width;
    unsigned int storage_height;

    unsigned int dst_x0;
    unsigned int dst_y0;
    unsigned int dst_x1;
    unsigned int dst_y1;
    unsigned int dst_w;
    unsigned int dst_h;

    unsigned int dirty_dy0;
    unsigned int dirty_dy1;
    unsigned int dy;

    int rebuild_map;

    if (!ps2vnc_hires_requires_raster_mapping() ||
        dst == NULL ||
        gs_framebuffer == NULL ||
        !rfb_dirty_valid ||
        desktop_width == 0 ||
        desktop_height == 0 ||
        desktop_output_width == 0 ||
        desktop_output_height == 0 ||
        OUTPUT_WIDTH == 0 ||
        OUTPUT_HEIGHT == 0 ||
        desktop_width > VNC_WIDTH ||
        desktop_height > VNC_HEIGHT ||
        rfb_dirty_y1 > desktop_height ||
        rfb_dirty_y0 >= rfb_dirty_y1 ||
        !region_fits_u32(
            desktop_output_x,
            desktop_output_y,
            desktop_output_width,
            desktop_output_height,
            OUTPUT_WIDTH,
            OUTPUT_HEIGHT)) {

        return -1;
    }

    storage_width =
        ps2vnc_hires_storage_width();

    storage_height =
        ps2vnc_hires_storage_height();

    if (storage_width == 0 ||
        storage_height == 0 ||
        storage_width > VNC_WIDTH ||
        storage_height > VNC_HEIGHT) {

        return -1;
    }

    rebuild_map =
        !map_valid ||
        cached_storage_width != storage_width ||
        cached_storage_height != storage_height ||
        cached_output_width != OUTPUT_WIDTH ||
        cached_output_height != OUTPUT_HEIGHT ||
        cached_desktop_width != desktop_width ||
        cached_desktop_height != desktop_height ||
        cached_output_x != desktop_output_x ||
        cached_output_y != desktop_output_y ||
        cached_output_w != desktop_output_width ||
        cached_output_h != desktop_output_height;

    if (rebuild_map) {
        unsigned int i;

        /*
         * Preserve F7B's hardware-proven physical->native rectangle arithmetic
         * exactly: floor the leading edge, ceil the trailing edge.
         */
        dst_x0 =
            (unsigned int)(
                ((u64)desktop_output_x * storage_width) /
                OUTPUT_WIDTH);

        dst_y0 =
            (unsigned int)(
                ((u64)desktop_output_y * storage_height) /
                OUTPUT_HEIGHT);

        dst_x1 =
            (unsigned int)(
                (((u64)(
                    desktop_output_x + desktop_output_width) *
                    storage_width) +
                  OUTPUT_WIDTH - 1) /
                 OUTPUT_WIDTH);

        dst_y1 =
            (unsigned int)(
                (((u64)(
                    desktop_output_y + desktop_output_height) *
                    storage_height) +
                  OUTPUT_HEIGHT - 1) /
                 OUTPUT_HEIGHT);

        if (dst_x1 > storage_width)
            dst_x1 = storage_width;

        if (dst_y1 > storage_height)
            dst_y1 = storage_height;

        if (dst_x0 >= dst_x1 ||
            dst_y0 >= dst_y1) {

            return -1;
        }

        dst_w = dst_x1 - dst_x0;
        dst_h = dst_y1 - dst_y0;

        if (dst_w > VNC_WIDTH ||
            dst_h > VNC_HEIGHT) {

            return -1;
        }

        /*
         * Exact F7B nearest-neighbor forward samples.
         */
        for (i = 0; i < dst_w; i++) {
            x_sample[i] =
                (unsigned int)(
                    ((u64)i * desktop_width) /
                    dst_w);
        }

        for (i = 0; i < dst_h; i++) {
            y_sample[i] =
                (unsigned int)(
                    ((u64)i * desktop_height) /
                    dst_h);
        }

        /*
         * Exact inverse boundaries used by F7C2:
         *
         *     ceil(source_edge * destination_size / source_size)
         */
        for (i = 0; i <= desktop_width; i++) {
            x_edge[i] =
                (unsigned int)(
                    (((u64)i * dst_w) +
                     desktop_width - 1) /
                    desktop_width);
        }

        for (i = 0; i <= desktop_height; i++) {
            y_edge[i] =
                (unsigned int)(
                    (((u64)i * dst_h) +
                     desktop_height - 1) /
                    desktop_height);
        }

        cached_storage_width = storage_width;
        cached_storage_height = storage_height;

        cached_output_width = OUTPUT_WIDTH;
        cached_output_height = OUTPUT_HEIGHT;

        cached_desktop_width = desktop_width;
        cached_desktop_height = desktop_height;

        cached_output_x = desktop_output_x;
        cached_output_y = desktop_output_y;
        cached_output_w = desktop_output_width;
        cached_output_h = desktop_output_height;

        cached_dst_x0 = dst_x0;
        cached_dst_y0 = dst_y0;
        cached_dst_x1 = dst_x1;
        cached_dst_y1 = dst_y1;
        cached_dst_w = dst_w;
        cached_dst_h = dst_h;

        map_valid = 1;

    } else {
        dst_x0 = cached_dst_x0;
        dst_y0 = cached_dst_y0;
        dst_x1 = cached_dst_x1;
        dst_y1 = cached_dst_y1;
        dst_w = cached_dst_w;
        dst_h = cached_dst_h;
    }

    /*
     * Ordinary live path begins here. No multiply/divide is needed below.
     */
    dirty_dy0 = y_edge[rfb_dirty_y0];
    dirty_dy1 = y_edge[rfb_dirty_y1];

    if (dirty_dy0 > dst_h)
        dirty_dy0 = dst_h;

    if (dirty_dy1 > dst_h)
        dirty_dy1 = dst_h;

    for (dy = dirty_dy0;
         dy < dirty_dy1;
         dy++) {

        unsigned int src_y;
        unsigned int src_x0;
        unsigned int src_x1;

        unsigned int dirty_dx0;
        unsigned int dirty_dx1;

        unsigned int dx;

        unsigned short *dst_row;
        const unsigned short *src_row;

        src_y = y_sample[dy];

        if (src_y >= desktop_height)
            return -1;

        if (!rfb_dirty_row_valid[src_y])
            continue;

        src_x0 = rfb_dirty_row_x0[src_y];
        src_x1 = rfb_dirty_row_x1[src_y];

        if (src_x1 <= src_x0 ||
            src_x1 > desktop_width) {

            return -1;
        }

        dirty_dx0 = x_edge[src_x0];
        dirty_dx1 = x_edge[src_x1];

        if (dirty_dx0 > dst_w)
            dirty_dx0 = dst_w;

        if (dirty_dx1 > dst_w)
            dirty_dx1 = dst_w;

        /*
         * A source span can legitimately disappear under downscaling.
         */
        if (dirty_dx0 >= dirty_dx1)
            continue;

        dst_row =
            &dst[
                ps2vnc_hires_mapped_storage_row(
                    dst_y0 + dy) *
                storage_width +
                dst_x0
            ];

        src_row =
            &gs_framebuffer[
                src_y * desktop_width
            ];

        for (dx = dirty_dx0;
             dx < dirty_dx1;
             dx++) {

            unsigned int src_x =
                x_sample[dx];

            if (src_x < src_x0 ||
                src_x >= src_x1 ||
                src_x >= desktop_width) {

                return -1;
            }

            dst_row[dx] = src_row[src_x];
        }

        SyncDCache(
            &dst_row[dirty_dx0],
            (unsigned char *)&dst_row[dirty_dx1]
        );
    }

    return 0;
}


/*
 * D17AL-F7C2:
 *
 * Native incremental publication follows the old proven HIRES double-buffer
 * ownership model, but with scaled dirty spans:
 *
 *   1. update only dirty native spans in hidden buffer;
 *   2. publish hidden buffer;
 *   3. swap front/back ownership;
 *   4. replay those same source dirty spans into the now-hidden old front.
 *
 * Step 4 keeps both presentation buffers at the same completed-frame baseline,
 * so the next incremental update may safely start from either allocation.
 */
static int ps2vnc_display_wait_presentation_memory_idle(
    GSGLOBAL *gsGlobal);



static int ps2vnc_publish_mapped_hires_dirty_frame(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    const unsigned short *gs_framebuffer)
{
    unsigned short *old_front;
    unsigned short *hidden;

    if (!ps2vnc_hires_requires_raster_mapping() ||
        gsGlobal == NULL ||
        tex == NULL ||
        gs_framebuffer == NULL ||
        hires_bg_buffer == NULL ||
        hires_bg_back_buffer == NULL ||
        !hires_bg_configured) {

        return -1;
    }

    /*
     * D17AL-F7C4:
     *
     * Restore F7C2's atomic hidden-buffer ownership. F7C3 demonstrated on
     * hardware that modifying the installed EE background in place makes
     * progressive pass consumption visible while moving large objects.
     *
     * F7C4 keeps the safe ownership model but removes the expensive divisions
     * from the dirty scaler itself.
     */
    old_front = hires_bg_buffer;
    hidden = hires_bg_back_buffer;

    if (hidden == old_front)
        return -1;

    /*
     * Build the completed dirty delta only in memory not currently referenced
     * by the installed gsHires presentation.
     */
    if (ps2vnc_update_mapped_hires_dirty_buffer(
            hidden,
            gs_framebuffer) < 0) {

        return -1;
    }

    tex->Mem = (u32 *)hidden;

    /*
     * Publish only after the hidden frame is complete. This retains the proven
     * F7C2 atomicity while the lookup-table experiment isolates scaler cost.
     */
    if (gsKit_hires_set_bg(gsGlobal, tex) < 0) {
        tex->Mem = (u32 *)old_front;
        return -1;
    }

    hires_bg_buffer = hidden;
    hires_bg_back_buffer = old_front;

    /*
     * The old front is hidden now. Replay the exact same dirty delta so either
     * allocation is a valid baseline for the next incremental update.
     */
    if (ps2vnc_update_mapped_hires_dirty_buffer(
            hires_bg_back_buffer,
            gs_framebuffer) < 0) {

        return -1;
    }

    live_direct_present_safe = 0;
    live_direct_present_buffer = NULL;
    live_linear_framebuffer_stale = 0;

    return 0;
}



static int ps2vnc_display_commit_update_hires(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    int direct_present_complete)
{
    unsigned short *old_front;

    /*
     * D17AL-F7C4 / F8E:
     *
     * Keep the authoritative CPU framebuffer and validated presentation
     * geometry. Map only completed dirty spans through cached coordinate maps
     * into the hidden presentation surface before atomic publication.
     */
    if (ps2vnc_hires_requires_raster_mapping()) {
        if (hires_mapped_source_framebuffer == NULL) {
            live_error_code = 208;
            return -1;
        }

        profile_dirty_work_start_tick =
            GetTimerSystemTime();

        profile_dirty_updates++;

        if (ps2vnc_publish_mapped_hires_dirty_frame(
                gsGlobal,
                tex,
                hires_mapped_source_framebuffer) < 0) {

            live_error_code = 209;
            return -1;
        }

        profile_dirty_work_ticks +=
            GetTimerSystemTime() -
            profile_dirty_work_start_tick;

        live_linear_framebuffer_stale = 0;
        return 0;
    }

    if (!direct_present_complete) {
        live_error_code = 204;
        return -1;
    }

    if (!rfb_dirty_valid) {
        live_error_code = 203;
        return -1;
    }

    profile_dirty_work_start_tick = GetTimerSystemTime();
    profile_dirty_updates++;

    /*
     * Hextile has completed the hidden frame. Make only the changed rows
     * DMA-visible before publishing that buffer.
     */
    if (sync_copy_hires_dirty_rows(
            live_direct_present_buffer,
            NULL) < 0) {
        live_error_code = 206;
        return -1;
    }

    tex->Mem = (u32 *)live_direct_present_buffer;

    if (gsKit_hires_set_bg(gsGlobal, tex) < 0) {
        live_error_code = 205;
        return -1;
    }

    old_front = hires_bg_buffer;
    hires_bg_buffer = live_direct_present_buffer;
    hires_bg_back_buffer = old_front;
    live_direct_present_buffer = hires_bg_back_buffer;

    /*
     * The old front is hidden now. Mirror only this completed incremental
     * delta into it so it is a valid base for the next server update.
     */
    if (sync_copy_hires_dirty_rows(
            live_direct_present_buffer,
            hires_bg_buffer) < 0) {
        live_error_code = 207;
        return -1;
    }

    profile_dirty_work_ticks +=
        GetTimerSystemTime() - profile_dirty_work_start_tick;

    return 0;
}

/*
 * Publish one successfully decoded Standard-backend incremental update.
 *
 * Standard has no decoder-direct presentation target. The ordinary decoder
 * keeps the linear GS16 framebuffer authoritative, and the proven Standard
 * renderer uploads the complete desktop texture whenever anything changed.
 */
static int ps2vnc_display_commit_update_standard(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex)
{
    if (gsGlobal == NULL || tex == NULL)
        return -1;

    if (!rfb_dirty_valid) {
        live_error_code = 203;
        return -1;
    }

    /*
     * This flag is a HIRES direct-write condition. Seeing it under Standard
     * would mean the supposedly authoritative linear framebuffer is unsafe.
     */
    if (live_linear_framebuffer_stale) {
        live_error_code = 209;
        return -1;
    }

    if (tex->Width != desktop_width ||
        tex->Height != desktop_height ||
        tex->Mem == NULL) {
        live_error_code = 210;
        return -1;
    }

    profile_dirty_work_start_tick = GetTimerSystemTime();
    profile_dirty_updates++;

    /*
     * Preserve the hardware-proven Test13 Standard policy: network updates
     * may be incremental, but any changed update causes one complete GS
     * texture upload.
     */
    gsKit_texture_upload(gsGlobal, tex);

    profile_dirty_work_ticks +=
        GetTimerSystemTime() - profile_dirty_work_start_tick;

    return 0;
}

static int ps2vnc_display_commit_update(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    int direct_present_complete)
{
    if (gsGlobal == NULL || tex == NULL)
        return -1;

    if (active_display_backend == PS2VNC_BACKEND_HIRES)
        return ps2vnc_display_commit_update_hires(
            gsGlobal,
            tex,
            direct_present_complete);

    if (active_display_backend == PS2VNC_BACKEND_STANDARD)
        return ps2vnc_display_commit_update_standard(
            gsGlobal,
            tex);

    return -1;
}


static int ps2vnc_display_wait_presentation_memory_idle(
    GSGLOBAL *gsGlobal);
static int ps2vnc_display_wait_texture_idle(
    GSGLOBAL *gsGlobal);

static int publish_hires_background(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *bg_tex,
    const unsigned short *gs_framebuffer)
{
    const unsigned int storage_width =
        ps2vnc_hires_storage_width();

    const unsigned int storage_height =
        ps2vnc_hires_storage_height();

    const unsigned int output_pixels =
        storage_width * storage_height;

    const unsigned int output_bytes =
        output_pixels * sizeof(unsigned short);

    unsigned int row;
    unsigned int i;

    if (hires_bg_buffer == NULL)
        return -1;

    if (desktop_width == 0 ||
        desktop_height == 0 ||
        !region_fits_u32(
            desktop_output_x,
            desktop_output_y,
            desktop_width,
            desktop_height,
            OUTPUT_WIDTH,
            OUTPUT_HEIGHT)) {

        return -1;
    }

    /*
     * The HIRES queues DMA directly from this EE-memory backing buffer.
     * Never modify it while the previous frame may still consume it.
     */
    if (hires_bg_configured &&
        ps2vnc_display_wait_presentation_memory_idle(gsGlobal) < 0)
        return -1;

    /*
     * Clear the complete active presentation storage before reconstruction.
     * 0x8000 is opaque black in the GS16 framebuffer format.
     */
    for (i = 0; i < output_pixels; i++)
        hires_bg_buffer[i] = 0x8000;

    /*
     * The RFB framebuffer remains tightly packed at desktop_width. Presentation
     * placement is expressed in physical-raster coordinates and is either
     * mapped into the active HIRES surface or copied through the 1:1 path.
     */
    if (ps2vnc_hires_requires_raster_mapping()) {
        unsigned int dst_x0;
        unsigned int dst_y0;
        unsigned int dst_x1;
        unsigned int dst_y1;
        unsigned int dst_w;
        unsigned int dst_h;
        unsigned int col;

        /*
         * F7B proved that calibration/RFB geometry remains authoritative in
         * physical-raster space while only the presentation rectangle is
         * converted into gsHires drawing/storage space. F8E applies that rule
         * from the active raster and presentation dimensions.
         *
         * Floor the leading edge and ceil the trailing edge so no physical
         * safe-area pixel is silently lost to integer truncation.
         */
        dst_x0 =
            (unsigned int)(
                ((u64)desktop_output_x * storage_width) /
                OUTPUT_WIDTH);

        dst_y0 =
            (unsigned int)(
                ((u64)desktop_output_y * storage_height) /
                OUTPUT_HEIGHT);

        dst_x1 =
            (unsigned int)(
                (((u64)(
                    desktop_output_x + desktop_output_width) *
                    storage_width) +
                  OUTPUT_WIDTH - 1) /
                 OUTPUT_WIDTH);

        dst_y1 =
            (unsigned int)(
                (((u64)(
                    desktop_output_y + desktop_output_height) *
                    storage_height) +
                  OUTPUT_HEIGHT - 1) /
                 OUTPUT_HEIGHT);

        if (dst_x1 > storage_width)
            dst_x1 = storage_width;

        if (dst_y1 > storage_height)
            dst_y1 = storage_height;

        if (dst_x0 >= dst_x1 || dst_y0 >= dst_y1)
            return -1;

        dst_w = dst_x1 - dst_x0;
        dst_h = dst_y1 - dst_y0;

        /*
         * Correctness-first nearest-neighbor conversion.
         *
         * F7B established the complete mapped-frame rebuild. Later F7C work
         * optimized incremental dirty publication without changing this
         * authoritative raster-to-presentation geometry.
         */
        for (row = 0; row < dst_h; row++) {
            unsigned int src_y =
                (unsigned int)(
                    ((u64)row * desktop_height) /
                    dst_h);

            unsigned short *dst =
                &hires_bg_buffer[
                    ps2vnc_hires_mapped_storage_row(
                        dst_y0 + row) *
                    storage_width +
                    dst_x0
                ];

            const unsigned short *src =
                &gs_framebuffer[
                    src_y * desktop_width
                ];

            for (col = 0; col < dst_w; col++) {
                unsigned int src_x =
                    (unsigned int)(
                        ((u64)col * desktop_width) /
                        dst_w);

                dst[col] = src[src_x];
            }
        }

    } else {
        /*
         * Exact 1:1 HIRES behavior: raster-sized presentation requires no
         * raster-to-presentation conversion.
         */
        for (row = 0; row < desktop_height; row++) {
            unsigned short *dst =
                &hires_bg_buffer[
                    hires_bg_storage_row(desktop_output_y + row) *
                    OUTPUT_WIDTH +
                    desktop_output_x
                ];

            const unsigned short *src =
                &gs_framebuffer[
                    row * desktop_width
                ];

            memcpy(
                dst,
                src,
                desktop_width * sizeof(unsigned short)
            );
        }
    }

    SyncDCache(
        hires_bg_buffer,
        (unsigned char *)hires_bg_buffer + output_bytes
    );

    if (!hires_bg_configured) {
        bg_tex->Mem = (u32 *)hires_bg_buffer;

        /*
         * Configure the active HIRES background exactly once.
         */
        if (gsKit_hires_set_bg(gsGlobal, bg_tex) < 0)
            return -1;

        hires_bg_configured = 1;
    }

    return 0;
}



/*
 * Publish one authoritative complete desktop after a full Raw receive.
 *
 * HIRES rebuilds its visible background and then makes the hidden direct
 * buffer coherent with that full frame. Standard's linear gs_framebuffer is
 * already authoritative, so publication is a conventional texture upload.
 */
static int ps2vnc_display_publish_full_desktop_hires(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    unsigned short *gs_framebuffer)
{
    /*
     * D17AL-F7C1 / F8E:
     *
     * Full Raw/recovery publication gets the same mapped double-buffer rule
     * as incremental updates. Do not mutate the displayed front.
     */
    if (ps2vnc_hires_requires_raster_mapping()) {
        unsigned int output_bytes;

        if (ps2vnc_publish_mapped_hires_hidden_frame(
                gsGlobal,
                tex,
                gs_framebuffer) < 0) {

            return -1;
        }

        /*
         * D17AL-F8G10D:
         *
         * A complete mapped publication has just atomically installed the
         * rebuilt hidden frame and made the previous front the new hidden
         * back buffer.
         *
         * Incremental mapped publication assumes BOTH EE presentation
         * buffers begin from the same authoritative full-frame baseline and
         * subsequently applies only each dirty delta to the hidden copy.
         *
         * Keep the new hidden buffer coherent now, before Hextile resumes.
         * This is the mapped equivalent of the existing full-frame
         * synchronization immediately below for ordinary 1:1 HIRES.
         */
        if (hires_bg_buffer == NULL ||
            hires_bg_back_buffer == NULL) {

            return -1;
        }

        output_bytes =
            ps2vnc_hires_storage_bytes();

        if (output_bytes == 0)
            return -1;

        memcpy(
            hires_bg_back_buffer,
            hires_bg_buffer,
            output_bytes
        );

        SyncDCache(
            hires_bg_back_buffer,
            (unsigned char *)hires_bg_back_buffer +
                output_bytes
        );

        live_direct_present_safe = 0;
        live_direct_present_buffer = NULL;
        live_linear_framebuffer_stale = 0;

        return 0;
    }


    if (publish_hires_background(
            gsGlobal,
            tex,
            gs_framebuffer) < 0)
        return -1;

    /*
     * Incremental HIRES operation requires both presentation buffers to
     * contain the same authoritative full frame before Hextile resumes.
     */
    if (hires_bg_back_buffer != NULL) {
        unsigned int output_bytes =
            ps2vnc_hires_storage_bytes();

        memcpy(
            hires_bg_back_buffer,
            hires_bg_buffer,
            output_bytes);

        SyncDCache(
            hires_bg_back_buffer,
            (unsigned char *)hires_bg_back_buffer + output_bytes);

        if (ps2vnc_hires_requires_raster_mapping())
            live_direct_present_buffer = NULL;
        else
            live_direct_present_buffer = hires_bg_back_buffer;
    }

    live_linear_framebuffer_stale = 0;
    return 0;
}

static int ps2vnc_display_publish_full_desktop_standard(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    unsigned short *gs_framebuffer)
{
    if (gsGlobal == NULL ||
        tex == NULL ||
        gs_framebuffer == NULL ||
        tex->Width != desktop_width ||
        tex->Height != desktop_height)
        return -1;

    /*
     * Standard decoding keeps this linear GS16 framebuffer authoritative.
     * Reassert the pointer and upload the complete replacement desktop.
     */
    tex->Mem = (u32 *)gs_framebuffer;

    debug_stage_id = DBG_STAGE_PRE_TEXTURE;
    gsKit_texture_upload(gsGlobal, tex);
    debug_stage_id = DBG_STAGE_POST_TEXTURE;

    live_linear_framebuffer_stale = 0;
    return 0;
}

static int ps2vnc_display_publish_full_desktop(
    GSGLOBAL *gsGlobal,
    GSTEXTURE *tex,
    unsigned short *gs_framebuffer)
{
    if (active_display_backend == PS2VNC_BACKEND_HIRES)
        return ps2vnc_display_publish_full_desktop_hires(
            gsGlobal, tex, gs_framebuffer);

    if (active_display_backend == PS2VNC_BACKEND_STANDARD)
        return ps2vnc_display_publish_full_desktop_standard(
            gsGlobal, tex, gs_framebuffer);

    return -1;
}

static int update_hires_background_region(
    GSGLOBAL *gsGlobal,
    const unsigned short *gs_framebuffer,
    unsigned int x,
    unsigned int y,
    unsigned int w,
    unsigned int h,
    int copy_pixels)
{
    unsigned int row;
    unsigned int row_bytes;

    if (hires_bg_buffer == NULL ||
        !hires_bg_configured)
        return -1;

    if (w == 0 || h == 0)
        return 0;

    if (!region_fits_u32(
            x, y, w, h,
            desktop_width,
            desktop_height))
        return -1;

    if (!region_fits_u32(
            desktop_output_x,
            desktop_output_y,
            desktop_width,
            desktop_height,
            OUTPUT_WIDTH,
            OUTPUT_HEIGHT))
        return -1;

    /*
     * One synchronization for the entire completed RFB update.
     */
    if (copy_pixels) {
        u64 t0 = GetTimerSystemTime();

        if (ps2vnc_display_wait_presentation_memory_idle(
                gsGlobal) < 0)
            return -1;

        profile_dirty_wait_ticks +=
            GetTimerSystemTime() - t0;
    }

    profile_dirty_work_start_tick = GetTimerSystemTime();
    profile_dirty_updates++;

    /*
     * TEST13H behavior retained for wide updates:
     *
     * Copy the complete affected DESKTOP rows because gs_framebuffer
     * is authoritative. Unlike old 13H, source and destination strides
     * differ, so each row must be copied separately.
     *
     * We still perform only one cache flush over the resulting physical
     * band. The flush harmlessly spans the margin gaps between rows.
     */
    if (w >= (desktop_width / 4)) {
        unsigned short *first_dst;
        unsigned short *last_dst_end;

        row_bytes =
            desktop_width * sizeof(unsigned short);

        if (hires_bg_is_field_separated()) {
            unsigned short *first[2] = { NULL, NULL };
            unsigned short *last[2] = { NULL, NULL };

            for (row = 0; row < h; row++) {
                unsigned int storage_row =
                    hires_bg_storage_row(desktop_output_y + y + row);
                unsigned int field =
                    storage_row >= (OUTPUT_HEIGHT / 2);
                unsigned short *dst =
                    &hires_bg_buffer[
                        storage_row * OUTPUT_WIDTH + desktop_output_x];
                const unsigned short *src =
                    &gs_framebuffer[(y + row) * desktop_width];

                if (copy_pixels)

                    memcpy(dst, src, row_bytes);

                if (first[field] == NULL)
                    first[field] = dst;
                last[field] = dst + desktop_width;
            }

            for (row = 0; row < 2; row++) {
                if (first[row] != NULL)
                    SyncDCache(first[row],
                               (unsigned char *)last[row]);
            }

            profile_dirty_work_ticks +=
                GetTimerSystemTime() - profile_dirty_work_start_tick;
            return 0;
        }

        first_dst =
            &hires_bg_buffer[
                (desktop_output_y + y) * OUTPUT_WIDTH +
                desktop_output_x
            ];

        for (row = 0; row < h; row++) {
            unsigned short *dst =
                &hires_bg_buffer[
                    hires_bg_storage_row(desktop_output_y + y + row) *
                    OUTPUT_WIDTH +
                    desktop_output_x
                ];

            const unsigned short *src =
                &gs_framebuffer[
                    (y + row) * desktop_width
                ];

            if (copy_pixels)

                memcpy(dst, src, row_bytes);
        }

        last_dst_end =
            &hires_bg_buffer[
                (desktop_output_y + y + h - 1) *
                OUTPUT_WIDTH +
                desktop_output_x +
                desktop_width
            ];

        SyncDCache(
            first_dst,
            (unsigned char *)last_dst_end
        );

        profile_dirty_work_ticks +=
            GetTimerSystemTime() - profile_dirty_work_start_tick;
        return 0;
    }

    /*
     * Narrow dirty regions retain the 13F row-local behavior.
     */
    row_bytes = w * sizeof(unsigned short);

    for (row = 0; row < h; row++) {
        unsigned short *dst =
            &hires_bg_buffer[
                hires_bg_storage_row(desktop_output_y + y + row) *
                OUTPUT_WIDTH +
                desktop_output_x +
                x
            ];

        const unsigned short *src =
            &gs_framebuffer[
                (y + row) * desktop_width +
                x
            ];

        if (copy_pixels)

            memcpy(dst, src, row_bytes);

        SyncDCache(
            dst,
            (unsigned char *)dst + row_bytes
        );
    }

    profile_dirty_work_ticks +=
        GetTimerSystemTime() - profile_dirty_work_start_tick;
    return 0;
}


static int ps2vnc_display_finish_draw(GSGLOBAL *gsGlobal);
static int ps2vnc_display_flip(GSGLOBAL *gsGlobal);

static int draw_recovery_status(
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
static int recover_rfb_connection(
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
static int recover_rfb_transport_for_display_rollback(
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


static int perform_manual_rfb_recovery(
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

static int screen_fit_width = VNC_WIDTH;
static int screen_fit_height = VNC_HEIGHT;
static int screen_fit_offset_x = 0;
static int screen_fit_offset_y = 0;
static int screen_fit_full_bypass = 0;


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
static int screen_fit_apply_preset(
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

typedef struct {
    int safe_width;
    int safe_height;
    int safe_x;
    int safe_y;
} ps2vnc_display_config_t;




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



static int ps2vnc_config_apply_display(
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

static int ps2vnc_display_config_loaded = 0;


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
static int ps2vnc_display_profile_capture_active(
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
static int ps2vnc_display_profile_resolve_candidate(
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


static int ps2vnc_display_transaction_begin_to_pi(
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


static int ps2vnc_display_transaction_commit_to_pi(void)
{
    return ps2vnc_management_post(
        PS2VNC_DISPLAY_TX_COMMIT_PATH,
        NULL,
        0);
}


static int ps2vnc_display_transaction_restore_to_pi(void)
{
    return ps2vnc_management_post(
        PS2VNC_DISPLAY_TX_RESTORE_PATH,
        NULL,
        0);
}


static int ps2vnc_display_transaction_restored_to_pi(void)
{
    return ps2vnc_management_post(
        PS2VNC_DISPLAY_TX_RESTORED_PATH,
        NULL,
        0);
}


static int ps2vnc_display_transaction_ack_to_pi(void)
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


static int ps2vnc_management_get_bounded_quiet(
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


static int ps2vnc_management_post_text(
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


static int ps2vnc_display_policy_refresh_from_pi(void)
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


static int ps2vnc_display_policy_lock_to_pi(
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


static int ps2vnc_display_policy_hide_to_pi(
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


static int ps2vnc_display_tx_profiles_equal(
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

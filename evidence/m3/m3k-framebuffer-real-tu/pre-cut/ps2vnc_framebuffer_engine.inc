
 int start_controller_thread(int sock)
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

int request_framebuffer(int sock, int incremental)
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
 void ps2vnc_display_direct_write_cancel(void);
 int ps2vnc_display_begin_update(void);
 int ps2vnc_display_finish_update_decode(void);


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
int receive_framebuffer_update(
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
int probe_extended_desktop_size(
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

void ps2vnc_display_direct_write_cancel(void)
{
    live_direct_present_safe = 0;
}

 int ps2vnc_display_begin_update(void)
{
    ps2vnc_display_direct_write_cancel();

    if (active_display_backend == PS2VNC_BACKEND_HIRES &&
        !ps2vnc_hires_requires_raster_mapping())
        live_direct_present_safe = 1;

    return 0;
}

 int ps2vnc_display_finish_update_decode(void)
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
 int publish_hires_background(
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
 int ps2vnc_display_wait_presentation_memory_idle(
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

 int ps2vnc_display_commit_update(
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


 int ps2vnc_display_wait_presentation_memory_idle(
    GSGLOBAL *gsGlobal);
 int ps2vnc_display_wait_texture_idle(
    GSGLOBAL *gsGlobal);

int publish_hires_background(
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

int ps2vnc_display_publish_full_desktop(
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


 int ps2vnc_display_finish_draw(GSGLOBAL *gsGlobal);
 int ps2vnc_display_flip(GSGLOBAL *gsGlobal);

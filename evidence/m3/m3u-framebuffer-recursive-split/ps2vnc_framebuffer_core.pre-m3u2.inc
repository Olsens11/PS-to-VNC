
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

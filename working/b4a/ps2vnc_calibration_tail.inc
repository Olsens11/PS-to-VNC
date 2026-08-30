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


int run_screen_fit_calibration(
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
int calculate_presented_desktop_geometry(
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
int ps2vnc_controller_pad_acquire(void)
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


int ps2vnc_controller_pad_release(void)
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
int ps2vnc_run_runtime_calibration(
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
        !pstvnc_video_mode_supports_backend(
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
GSGLOBAL *ps2vnc_display_create(void)
{
    ps2vnc_backend_t backend =
        pstvnc_video_mode_default_backend(active_video_mode);
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
int ps2vnc_display_wait_presentation_memory_idle(
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
int ps2vnc_display_wait_texture_idle(
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
int ps2vnc_display_finish_draw(GSGLOBAL *gsGlobal)
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
int ps2vnc_display_flip(GSGLOBAL *gsGlobal)
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

int ps2vnc_display_destroy(GSGLOBAL **display)
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
int ps2vnc_display_prepare_desktop(
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
int ps2vnc_display_draw_desktop(
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

void ps2vnc_display_prepare_osk(
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

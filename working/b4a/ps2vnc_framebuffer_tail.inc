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

    pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_PRE_TEXTURE);
    gsKit_texture_upload(gsGlobal, tex);
    pstvnc_debug_set_stage(PSTVNC_DEBUG_STAGE_POST_TEXTURE);

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

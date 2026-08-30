static ps2vnc_backend_t ps2vnc_video_mode_default_backend(
    const ps2vnc_video_mode_t *mode)
{
    if (mode == NULL)
        return PS2VNC_BACKEND_NONE;

    if (!ps2vnc_video_mode_supports_backend(
            mode, mode->recommended_backend))
        return PS2VNC_BACKEND_NONE;

    return mode->recommended_backend;
}

static const ps2vnc_video_mode_t *ps2vnc_video_mode_by_name(
    const char *name)
{
    unsigned int i;

    if (name == NULL || name[0] == '\0')
        return NULL;

    for (i = 0; i < VIDEO_MODE_COUNT; i++) {
        if (strcmp(name, video_modes[i]->name) == 0)
            return video_modes[i];
    }

    return NULL;
}

/* Built-in fallback when no valid persisted startup mode exists. */
static const ps2vnc_video_mode_t *active_video_mode = &video_mode_1080i;
static ps2vnc_backend_t active_display_backend = PS2VNC_BACKEND_NONE;


/*
 * D17AL-F8G11B:
 *
 * gsHires CT16S FRAME/DISPFB presentation width is a 64-pixel-unit
 * hardware contract.  The mode registry records intended drawing geometry;
 * the renderer derives the compatible width without changing physical
 * raster timing or Standard-backend semantics.
 *
 * Keep horizontal width compatibility separate from vertical full-frame
 * geometry.  This distinction is essential for interlaced GS_FRAME modes:
 * a 720x288 PAL gsHires drawing surface represents a 720x576 full frame.
 * Narrowing that surface horizontally must not collapse its EE backing from
 * 576 field-separated raster rows to 288 rows.
 */
static unsigned int ps2vnc_hires_effective_width(void)
{
    unsigned int width;

    if (active_video_mode == NULL)
        return 0;

    width = active_video_mode->hires_width;

    if (width == 0)
        return 0;

    /*
     * No current HIRES mode is narrower than one GS FBW unit. Preserve a
     * defensive nonzero result if a future diagnostic ever introduces one.
     */
    if (width < 64u)
        return width;

    return (width / 64u) * 64u;
}


static u64 ps2vnc_hires_full_frame_height(void)
{
    u64 height;

    if (active_video_mode == NULL ||
        active_video_mode->hires_height == 0) {

        return 0;
    }

    height =
        (u64)active_video_mode->hires_height;

    if (active_video_mode->interlace == GS_INTERLACED &&
        active_video_mode->field == GS_FRAME) {

        height *= 2u;
    }

    return height;
}


static int ps2vnc_hires_requires_horizontal_mapping(void)
{
    unsigned int effective_width;

    if (active_video_mode == NULL ||
        active_video_mode->raster_width == 0) {

        return 0;
    }

    effective_width =
        ps2vnc_hires_effective_width();

    if (effective_width == 0)
        return 0;

    return
        effective_width !=
            active_video_mode->raster_width;
}


static int ps2vnc_hires_requires_vertical_mapping(void)
{
    u64 full_frame_height;

    if (active_video_mode == NULL ||
        active_video_mode->raster_height == 0) {

        return 0;
    }

    full_frame_height =
        ps2vnc_hires_full_frame_height();

    if (full_frame_height == 0)
        return 0;

    return
        full_frame_height !=
            (u64)active_video_mode->raster_height;
}


static int ps2vnc_hires_requires_raster_mapping(void)
{
    return
        ps2vnc_hires_requires_horizontal_mapping() ||
        ps2vnc_hires_requires_vertical_mapping();
}


/*
 * EE presentation backing follows the effective horizontal renderer width.
 * Vertical storage remains a separate property:
 *
 *   - true vertical presentation scaling uses the explicit HIRES height;
 *   - otherwise retain full physical-raster height, including the existing
 *     field-separated representation of interlaced GS_FRAME modes.
 */
static unsigned int ps2vnc_hires_storage_width(void)
{
    return ps2vnc_hires_effective_width();
}


static unsigned int ps2vnc_hires_storage_height(void)
{
    if (active_video_mode == NULL)
        return 0;

    if (ps2vnc_hires_requires_vertical_mapping())
        return active_video_mode->hires_height;

    return active_video_mode->raster_height;
}


static unsigned int ps2vnc_hires_storage_bytes(void)
{
    return
        ps2vnc_hires_storage_width() *
        ps2vnc_hires_storage_height() *
        sizeof(unsigned short);
}

#define OUTPUT_WIDTH   (active_video_mode->raster_width)
#define OUTPUT_HEIGHT  (active_video_mode->raster_height)
#define VNC_WIDTH      RFB_CAPACITY_WIDTH
#define VNC_HEIGHT     RFB_CAPACITY_HEIGHT

/*
 * Convert physical output-raster coordinates into the active renderer's
 * drawing space.
 *
 * Physical raster, renderer framebuffer, and RFB desktop geometry are
 * separate coordinate spaces. Each backend supplies its own draw dimensions.
 */
static int ps2vnc_display_logical_desktop_size(
    unsigned int output_width,
    unsigned int output_height,
    unsigned int *width,
    unsigned int *height)
{
    if (active_video_mode == NULL || width == NULL || height == NULL)
        return -1;

    if (active_display_backend == PS2VNC_BACKEND_HIRES) {
        *width = output_width;
        *height = output_height;
    } else if (active_display_backend == PS2VNC_BACKEND_STANDARD) {
        *width = active_video_mode->standard_logical_width;
        *height = active_video_mode->standard_logical_height;
    } else {
        return -1;
    }

    if (*width == 0 || *height == 0 ||
        *width > VNC_WIDTH || *height > VNC_HEIGHT)
        return -1;

    return 0;
}

static float ps2vnc_display_output_x(float x)
{
    unsigned int draw_width;

    if (active_display_backend == PS2VNC_BACKEND_STANDARD)
        draw_width = active_video_mode->standard_fb_width;
    else if (active_display_backend == PS2VNC_BACKEND_HIRES)
        draw_width = ps2vnc_hires_effective_width();
    else
        return x;

    if (draw_width == 0 || OUTPUT_WIDTH == 0)
        return x;

    return x * (float)draw_width / (float)OUTPUT_WIDTH;
}

static float ps2vnc_display_output_y(float y)
{
    unsigned int draw_height;

    if (active_display_backend == PS2VNC_BACKEND_STANDARD)
        draw_height = active_video_mode->standard_fb_height;
    else if (active_display_backend == PS2VNC_BACKEND_HIRES)
        draw_height = active_video_mode->hires_height;
    else
        return y;

    if (draw_height == 0 || OUTPUT_HEIGHT == 0)
        return y;

    return y * (float)draw_height / (float)OUTPUT_HEIGHT;
}

/*
 * TEST13J-D3-A1:
 * Runtime RFB framebuffer geometry.
 *
 * Physical GS output remains fixed at 1280x720.
 */
static unsigned int desktop_width = VNC_WIDTH;
static unsigned int desktop_height = VNC_HEIGHT;

/*
 * Position of the 1:1 remote desktop inside the fixed 1280x720
 * physical HIRES output.
 */
static unsigned int desktop_output_x = 0;
static unsigned int desktop_output_y = 0;
static unsigned int desktop_output_width = VNC_WIDTH;
static unsigned int desktop_output_height = VNC_HEIGHT;

static unsigned short *hires_bg_buffer = NULL;
static unsigned short *hires_bg_back_buffer = NULL;
static unsigned short *live_direct_present_buffer = NULL;

/*
 * D17AL-F7B:
 *
 * For the isolated 480p native-surface path, Hextile continues decoding into
 * the ordinary authoritative CPU framebuffer. The presentation backend then
 * maps that complete image into 704x462.
 */
static unsigned short *hires_mapped_source_framebuffer = NULL;
static int hires_bg_configured = 0;
static int live_direct_present_safe = 0;
static int live_linear_framebuffer_stale = 0;
static unsigned int hires_bg_storage_row(unsigned int raster_y);

/*
 * Before calibration we accept whatever <=1280x720 geometry TigerVNC
 * currently has. Once the safe desktop is finalized, recovery sessions
 * will be required to return at exactly that geometry.
 */
static int desktop_geometry_locked = 0;
/*
 * PS2VNC private-link bootstrap contract.
 *
 * The installer will provide the same dedicated Ethernet topology on every
 * supported installation.  The PS2 must know this address before it can
 * retrieve the general configuration file, so it cannot itself live inside
 * that remotely hosted configuration.
 *
 * Variable post-bootstrap settings can be added to ps2vnc.conf later.
 */
#define PS2VNC_BOOTSTRAP_PI_IP      "192.168.50.1"
#define PS2VNC_CONFIG_PORT          5959
#define PS2VNC_CONFIG_PATH          "/ps2vnc.conf"
#define PS2VNC_DISPLAY_PATH         "/display"
#define PS2VNC_DISPLAY_MODE_PATH    "/display-mode"
#define PS2VNC_DISPLAY_LOCK_PATH    "/display-lock"
#define PS2VNC_DISPLAY_UI_PATH      "/display-ui"

#define PS2VNC_DISPLAY_TX_PATH \
    "/display-transaction"
#define PS2VNC_DISPLAY_TX_BEGIN_PATH \
    "/display-transaction/begin"
#define PS2VNC_DISPLAY_TX_COMMIT_PATH \
    "/display-transaction/commit"
#define PS2VNC_DISPLAY_TX_RESTORE_PATH \
    "/display-transaction/restore"
#define PS2VNC_DISPLAY_TX_RESTORED_PATH \
    "/display-transaction/restored"
#define PS2VNC_DISPLAY_TX_ACK_PATH \
    "/display-transaction/ack"

#define PS2VNC_DISPLAY_CONTROL_PENDING_PATH \
    "/display-control/pending"
#define PS2VNC_DISPLAY_CONTROL_RESULT_PATH \
    "/display-control/result"

#define PS2VNC_DISPLAY_CONTROL_MAX_BYTES 768
#define PS2VNC_REMOTE_CONTROL_POLL_US     500000u

#define PS2VNC_REMOTE_CONTROL_GET_BUDGET_US 100000u
#define PS2VNC_HTTP_MAX_HEADER      2048
#define PS2VNC_DISPLAY_TX_MAX_BYTES 2048

#define VNC_PORT                    5900

/* X11 keysyms used by the RFB KeyEvent / on-screen keyboard. */
#define XK_BACKSPACE 0xFF08
#define XK_TAB       0xFF09
#define XK_RETURN    0xFF0D
#define XK_ESCAPE    0xFF1B
#define XK_CONTROL_L 0xFFE3
#define XK_ALT_L     0xFFE9
#define XK_LEFT       0xFF51
#define XK_UP         0xFF52
#define XK_RIGHT      0xFF53
#define XK_DOWN       0xFF54
#define XK_DELETE    0xFFFF

/*
 * Test11L extended OSK keysyms.
 */
#ifndef XK_SHIFT_L
#define XK_SHIFT_L    0xFFE1
#endif

#ifndef XK_CAPS_LOCK
#define XK_CAPS_LOCK  0xFFE5
#endif

#ifndef XK_HOME
#define XK_HOME       0xFF50
#endif

#ifndef XK_LEFT
#define XK_LEFT       0xFF51
#endif

#ifndef XK_UP
#define XK_UP         0xFF52
#endif

#ifndef XK_RIGHT
#define XK_RIGHT      0xFF53
#endif

#ifndef XK_DOWN
#define XK_DOWN       0xFF54
#endif

#ifndef XK_PAGE_UP
#define XK_PAGE_UP    0xFF55
#endif

#ifndef XK_PAGE_DOWN
#define XK_PAGE_DOWN  0xFF56
#endif

#ifndef XK_END
#define XK_END        0xFF57
#endif

#ifndef XK_PRINT
#define XK_PRINT      0xFF61
#endif

#ifndef XK_INSERT
#define XK_INSERT     0xFF63
#endif

#ifndef XK_PAUSE
#define XK_PAUSE      0xFF13
#endif

#ifndef XK_F1
#define XK_F1         0xFFBE
#define XK_F2         0xFFBF
#define XK_F3         0xFFC0
#define XK_F4         0xFFC1
#define XK_F5         0xFFC2
#define XK_F6         0xFFC3
#define XK_F7         0xFFC4
#define XK_F8         0xFFC5
#define XK_F9         0xFFC6
#define XK_F10        0xFFC7
#define XK_F11        0xFFC8
#define XK_F12        0xFFC9
#endif

#define OSK_WIDTH    600
#define OSK_HEIGHT   178
#define OSK_ROWS     5

/*
 * Controller layout:
 *
 * Keyboard hidden:
 *   Left stick / D-pad = mouse
 *   X                  = left click
 *   Select             = open keyboard
 *   Square             = Backspace shortcut
 *   Start              = Enter shortcut
 *   R1                 = Tab shortcut
 *
 * Keyboard visible:
 *   D-pad              = select key
 *   X                  = type selected key
 *   Triangle           = one-shot Shift layer toggle
 *   CTRL / ALT keys    = one-shot real modifiers
 *   Square             = Backspace
 *   Start              = Enter
 *   R1                 = Tab
 *   Circle / Select    = close keyboard
 *
 * Shift is LOCAL to the OSK.  We send the final capital/symbol keysym
 * directly rather than synthesizing Shift + lowercase, which avoids the
 * CapsLock reconciliation behavior observed in TigerVNC.
 */

extern unsigned char DEV9_irx[];
extern unsigned int size_DEV9_irx;

extern unsigned char SMAP_irx[];
extern unsigned int size_SMAP_irx;

extern unsigned char NETMAN_irx[];
extern unsigned int size_NETMAN_irx;

/* libpad DMA area: 256 bytes, 64-byte aligned. */
static unsigned char pad_area[256] __attribute__((aligned(64)));

/* Dedicated EE input thread. */
#define CONTROLLER_STACK_SIZE 8192
static unsigned char controller_stack[CONTROLLER_STACK_SIZE]
    __attribute__((aligned(16)));

static int controller_thread_id = -1;
/*
 * Test26 rule: after the controller thread starts, ONLY the main/render
 * thread is allowed to call send()/recv() on the VNC TCP socket.
 *
 * The controller thread queues complete RFB PointerEvent/KeyEvent messages
 * here.  The main thread drains the queue while polling the server socket.
 * This avoids not only simultaneous socket calls, but also moving ownership
 * of the same lwIP socket back and forth between EE threads.
 */
#define RFB_OUT_QUEUE_CAPACITY 256
#define RFB_OUT_MESSAGE_MAX    48

typedef struct {
    unsigned int len;
    unsigned char data[RFB_OUT_MESSAGE_MAX];
} rfb_out_message_t;

static rfb_out_message_t rfb_out_queue[RFB_OUT_QUEUE_CAPACITY];
static volatile unsigned int rfb_out_head = 0;
static volatile unsigned int rfb_out_tail = 0;
static volatile unsigned int rfb_out_count = 0;
static int rfb_queue_sema = -1;

/*
 * Buffered TCP receive layer.
 *
 * Earlier builds called recv() for every RFB structure and, for Raw
 * rectangles, effectively once per framebuffer row.  During rapid window
 * dragging that can mean a very high rate of tiny recv() calls.
 *
 * Test28 receives larger chunks from PS2SDK/lwIP into this private buffer,
 * then satisfies all exact-length RFB reads with memcpy().  This both
 * reduces socket-call pressure and makes protocol structure boundaries
 * independent of individual recv() returns.
 */
#define RFB_RX_BUFFER_SIZE 32768
static unsigned char rfb_rx_buffer[RFB_RX_BUFFER_SIZE];
static unsigned int rfb_rx_pos = 0;
static unsigned int rfb_rx_end = 0;
static volatile unsigned int rfb_rx_syscalls = 0;

/*
 * Test31 receive/header sentinels retained for lightweight diagnostics.
 * The RX buffer is prefilled with A5 and rectangle headers with 5A so a
 * short/incomplete copy remains visible in the normal debug state.
 */
#define TEST31_RX_SENTINEL 0xA5
#define TEST31_HDR_SENTINEL 0x5A

static volatile int rfb_last_recv_n = 0;
static volatile unsigned int rfb_last_recv_call = 0;

static volatile unsigned int live_hdr_start_pos = 0;
static volatile unsigned int live_hdr_start_end = 0;
static volatile unsigned int live_hdr_start_avail = 0;
static volatile unsigned int live_hdr_end_pos = 0;
static volatile unsigned int live_hdr_end_end = 0;
static volatile int live_hdr_recv_n = 0;
static volatile unsigned int live_hdr_recv_call = 0;
static volatile unsigned char live_hdr_raw[12];

/* Raw FramebufferUpdate header tracing: type + pad + rectangle count. */
static volatile unsigned char live_update_raw[4];
static volatile unsigned int live_update_start_pos = 0;
static volatile unsigned int live_update_start_end = 0;
static volatile unsigned int live_update_end_pos = 0;
static volatile unsigned int live_update_end_end = 0;
static volatile unsigned int live_update_recv_call = 0;



/*
 * Test29 isolation mode:
 * The initial full frame is still stored and displayed normally.  Every
 * subsequent Raw rectangle is received and parsed, but its pixel payload is
 * discarded into this row scratch buffer.  No live RFB/GS framebuffer writes
 * and no live gsKit texture uploads occur.
 *
 * If the RFB parser still corrupts under violent Windows-side window dragging,
 * the fault is upstream of framebuffer writes / RGB565 conversion / texture
 * upload.  If it survives, one of those operations is implicated.
 */
#define TEST29_DISCARD_LIVE_PIXELS 0
static unsigned char test29_row_scratch[VNC_WIDTH * 2];

/*
 * TEST13F:
 * Bounding box of all Raw rectangles in the most recently completed
 * RFB FramebufferUpdate. x1/y1 are exclusive.
 */
static unsigned int rfb_dirty_x0 = 0;
static unsigned int rfb_dirty_y0 = 0;
static unsigned int rfb_dirty_x1 = 0;
static unsigned int rfb_dirty_y1 = 0;
static int rfb_dirty_valid = 0;

/*
 * TEST15D3-A3-A3:
 * Keep a conservative dirty X span for each desktop row in addition to the
 * historical whole-update bounding box.  Multiple RFB rectangles touching
 * the same row merge naturally.
 */
static unsigned int rfb_dirty_row_x0[VNC_HEIGHT];
static unsigned int rfb_dirty_row_x1[VNC_HEIGHT];
static unsigned char rfb_dirty_row_valid[VNC_HEIGHT];

static void rfb_dirty_reset(void)
{
    rfb_dirty_valid = 0;
    memset(
        rfb_dirty_row_valid,
        0,
        sizeof(rfb_dirty_row_valid)
    );
}

static void rfb_dirty_mark_rect(
    unsigned int x,
    unsigned int y,
    unsigned int w,
    unsigned int h)
{
    unsigned int row;

    if (w == 0 || h == 0)
        return;

    if (!rfb_dirty_valid) {
        rfb_dirty_x0 = x;
        rfb_dirty_y0 = y;
        rfb_dirty_x1 = x + w;
        rfb_dirty_y1 = y + h;
        rfb_dirty_valid = 1;
    } else {
        if (x < rfb_dirty_x0)
            rfb_dirty_x0 = x;
        if (y < rfb_dirty_y0)
            rfb_dirty_y0 = y;
        if (x + w > rfb_dirty_x1)
            rfb_dirty_x1 = x + w;
        if (y + h > rfb_dirty_y1)
            rfb_dirty_y1 = y + h;
    }

    for (row = y; row < y + h; row++) {
        if (!rfb_dirty_row_valid[row]) {
            rfb_dirty_row_x0[row] = x;
            rfb_dirty_row_x1[row] = x + w;
            rfb_dirty_row_valid[row] = 1;
        } else {
            if (x < rfb_dirty_row_x0[row])
                rfb_dirty_row_x0[row] = x;
            if (x + w > rfb_dirty_row_x1[row])
                rfb_dirty_row_x1[row] = x + w;
        }
    }
}


/*
 * Runtime calibration temporarily transfers libpad ownership from the
 * controller thread to the main/render thread.
 *
 * requested is written by the main thread. ack is written only by the
 * controller thread after it has stopped calling padGetState()/padRead().
 *
 * G2-C installs only the dormant ownership boundary. No current UI action
 * requests a handoff yet, so existing runtime behavior remains unchanged.
 */
static volatile int controller_pad_pause_requested = 0;
static volatile int controller_pad_pause_ack = 0;

static volatile int controller_thread_error = 0;

/*
 * TEST12B CONNECTION RECOVERY
 *
 * Controller input keeps running during a Pi/VNC outage, but normal
 * RFB input events are discarded until a complete replacement RFB
 * session is established.
 */
#define RECONNECT_TIMEOUT_SECONDS 10
#define RECONNECT_MAX_ATTEMPTS    10

static volatile int rfb_connected = 0;
static volatile int exit_requested = 0;

/*
 * D4 startup override:
 * hold L1 + R1 while PS2VNC starts to force interactive display
 * calibration even when a valid stored display preset was loaded.
 */
static int force_screen_fit_calibration = 0;

static int controller_sock = -1;

/* Last live RFB failure context, displayed if the render loop exits. */
static volatile int live_error_code = 0;
static volatile unsigned int live_message_type = 0;
static volatile unsigned int live_rect_count = 0;
static volatile unsigned int live_rect_index = 0;
static volatile unsigned int live_rect_x = 0;
static volatile unsigned int live_rect_y = 0;
static volatile unsigned int live_rect_w = 0;
static volatile unsigned int live_rect_h = 0;
static volatile unsigned int live_encoding = 0;

/*
 * Test27 diagnostic transport.
 *
 * There is NO debug/watchdog EE thread in this build.  The main/render
 * thread owns every PS2IP socket operation in the program:
 *
 *   - VNC TCP send/recv
 *   - low-rate UDP diagnostic sendto
 *
 * The controller thread only writes complete RFB input messages into RAM.
 * This lets us test whether merely having another EE thread inside PS2IP
 * was contributing to the receive corruption seen in test26.
 */
#define DEBUG_UDP_PORT 5999
static int debug_udp_sock = -1;
static struct sockaddr_in debug_udp_addr;
static volatile unsigned int debug_frame_counter = 0;
static volatile unsigned int debug_stage_id = 0;
static volatile int debug_changed_bytes = 0;

/* D17AL-F8J2-B4A hard-transition envelope phase, 0 = idle. */
static volatile unsigned int debug_transition_phase = 0;

/* TEST15C1 measurement-only live-pipeline profiler. */
static u64 profile_present_sync_ticks = 0;
static u64 profile_flip_ticks = 0;
static u64 profile_rfb_ticks = 0;
static u64 profile_rx_wait_ticks = 0;
static u64 profile_hextile_ticks = 0;
static u64 profile_hextile_rx_wait_ticks = 0;
static int profile_hextile_active = 0;
static u64 profile_dirty_wait_ticks = 0;
static u64 profile_dirty_work_ticks = 0;
static u64 profile_rfb_start_tick = 0;
static u64 profile_dirty_work_start_tick = 0;
static unsigned int profile_rfb_updates = 0;
static unsigned int profile_dirty_updates = 0;
static unsigned int profile_changed_bytes = 0;
static unsigned int profile_raw_rects = 0;
static unsigned int profile_hextile_rects = 0;
static volatile unsigned int controller_loop_counter = 0;
static unsigned int debug_idle_poll_counter = 0;

/*
 * TEST15E3H controller-state mirror.
 *
 * The controller thread writes only these scalar diagnostics.  UDP remains
 * owned exclusively by the main/RFB thread through debug_emit_now().
 */
static volatile unsigned int debug_hotkey_mask = 0;
static volatile unsigned int debug_hotkey_stable_polls = 0;
static volatile unsigned int debug_hotkey_settle_target = 0;
/* Runtime action NONE is numerically zero; avoid enum declaration order. */
static volatile int debug_hotkey_action = 0;
static volatile int debug_hotkey_trigger = 0;
static volatile unsigned int debug_hotkey_hold_polls = 0;
static volatile int debug_hotkey_latched = 0;
static volatile unsigned int debug_hotkey_quarantine = 0;

enum {
    DBG_STAGE_STARTUP = 0,
    DBG_STAGE_NET_READY,
    DBG_STAGE_GS_INIT,
    DBG_STAGE_INITIAL_TEXTURE,
    DBG_STAGE_CONTROLLER_START,
    DBG_STAGE_LOOP_BEGIN,
    DBG_STAGE_PRE_DRAW,
    DBG_STAGE_POST_DRAW,
    DBG_STAGE_PRE_FLIP,
    DBG_STAGE_POST_FLIP,
    DBG_STAGE_PRE_QUEUE,
    DBG_STAGE_POST_QUEUE,
    DBG_STAGE_PRE_REQUEST,
    DBG_STAGE_POST_REQUEST,
    DBG_STAGE_WAIT_RFB,
    DBG_STAGE_POST_RECEIVE,
    DBG_STAGE_PRE_TEXTURE,
    DBG_STAGE_POST_TEXTURE,
    DBG_STAGE_CONTROLLER_ERROR,
    DBG_STAGE_REQUEST_ERROR,
    DBG_STAGE_RECEIVE_ERROR
};

/* On-screen keyboard state shared between controller and renderer. */
static volatile int osk_visible = 0;
static volatile int osk_row = 1;
static volatile int osk_col = 0;
static volatile int osk_shift = 0;
static volatile int osk_ctrl = 0;
static volatile int osk_alt = 0;
static volatile int osk_page = 0;
static volatile unsigned int osk_generation = 1;

/*
 * TEST15E2-D2A local display selector.
 * The controller publishes only a semantic request.  The main RFB/render
 * thread remains the sole owner of configuration fetches and mode changes.
 */
/*
 * Display-mode requests cross from the controller thread to the main
 * RFB/render thread as registry indices plus one. Zero remains "none".
 * This keeps the request channel scalar/atomic without baking mode names
 * into the controller.
 */
#define DISPLAY_MODE_REQUEST_NONE 0

static int display_menu_selection_for_mode(
    const ps2vnc_video_mode_t *mode)
{
    unsigned int i;

    for (i = 0; i < VIDEO_MODE_COUNT; i++) {
        if (video_modes[i] == mode)
            return (int)i;
    }

    return 0;
}

static const ps2vnc_video_mode_t *display_menu_mode_at_selection(
    int selection)
{
    if (selection < 0 ||
        (unsigned int)selection >= VIDEO_MODE_COUNT)
        return NULL;

    return video_modes[selection];
}

static int display_mode_request_for_selection(int selection)
{
    if (selection < 0 ||
        (unsigned int)selection >= VIDEO_MODE_COUNT)
        return DISPLAY_MODE_REQUEST_NONE;

    return selection + 1;
}

static const ps2vnc_video_mode_t *display_mode_request_target(
    int request)
{
    int selection = request - 1;

    return display_menu_mode_at_selection(selection);
}

static volatile int display_menu_visible = 0;
static volatile int display_menu_selection = 0;
static volatile int display_menu_error = 0;
static volatile int display_mode_request = DISPLAY_MODE_REQUEST_NONE;

/*
 * A calibration request is semantically distinct from a mode switch even
 * though both carry the registry selection as index+1.  Same-mode X publishes
 * only this channel; it must never enter ps2vnc_switch_display_mode().
 */

static volatile int display_calibration_request =
    DISPLAY_MODE_REQUEST_NONE;


enum {
    REMOTE_DISPLAY_IDLE = 0,
    REMOTE_DISPLAY_SWITCH_REQUESTED,
    REMOTE_DISPLAY_SWITCHING,
    REMOTE_DISPLAY_ACTIVE_TEMP,
    REMOTE_DISPLAY_RESTORING
};

static volatile int remote_display_state =
    REMOTE_DISPLAY_IDLE;

/*
 * F8J1C:
 * Successful remote rollback is machine-owned and must return to the
 * ordinary UI underlay.  Legacy human transaction callers still invoke
 * display_confirm_return_to_active_menu(); consume that call once instead
 * of duplicating remote special-cases at every rollback exit.
 */
static volatile int
    remote_display_suppress_next_active_menu_return = 0;

static volatile int display_mode_request_remote = 0;

static unsigned int remote_display_txid = 0;
static unsigned int remote_display_lease_seconds = 0;
static unsigned int remote_display_error_code = 0;

/* F8J2-B2 diagnostic: exact failure-return ordinal inside
 * ps2vnc_switch_display_mode(). */
static volatile unsigned int display_switch_failure_stage = 0;

static u64 remote_display_lease_started_tick = 0;
static u64 remote_display_last_poll_tick = 0;

static const ps2vnc_video_mode_t *
    remote_display_requested_mode = NULL;

static const ps2vnc_video_mode_t *
    remote_display_baseline_mode = NULL;


/*
 * D17AL-F2 Display Settings UI.
 *
 * Tabs are presentation-only. Registry indices remain the stable
 * display-mode request / transaction identity.
 */
enum {
    DISPLAY_TAB_TV_DTV = 0,
    DISPLAY_TAB_VGA_60,
    DISPLAY_TAB_VGA_OTHER,
    DISPLAY_TAB_ADVANCED,
    DISPLAY_TAB_COUNT
};

enum {
    DISPLAY_POLICY_REQUEST_NONE = 0,
    DISPLAY_POLICY_REQUEST_REFRESH,
    DISPLAY_POLICY_REQUEST_LOCK,
    DISPLAY_POLICY_REQUEST_HIDE_LOCKED
};

static volatile int display_menu_tab = DISPLAY_TAB_TV_DTV;

/*
 * Remember the most recent registry selection in each mode tab.
 * ADVANCED has settings rows rather than a mode selection.
 */
static volatile int display_menu_tab_selection[3] = {
    -1, -1, -1
};

/*
 * Policy cache.
 *
 * These settings deliberately do NOT participate in complete display
 * transaction identity:
 *
 *   locked              = user/menu admission policy
 *   hide_locked_modes   = menu presentation preference
 *
 * Missing config values mean false for backwards compatibility.
 */
static volatile unsigned char
    display_mode_locked[VIDEO_MODE_COUNT];

static volatile int display_hide_locked_modes = 0;
static volatile int display_policy_loaded = 0;
static volatile int display_policy_error = 0;

/*
 * Controller -> main-thread semantic policy request.
 *
 * The controller never performs management HTTP.
 */
static volatile int display_policy_request =
    DISPLAY_POLICY_REQUEST_NONE;

static volatile int display_policy_request_selection = -1;
static volatile int display_policy_request_value = 0;

static void display_menu_open_for_active(int parent_system);

#define DISPLAY_CONFIRM_TIMEOUT_SECONDS 30u

enum {
    DISPLAY_CONFIRM_GO_BACK = 0,
    DISPLAY_CONFIRM_KEEP,
    DISPLAY_CONFIRM_ITEM_COUNT
};

enum {
    DISPLAY_CONFIRM_ACTION_NONE = 0,
    DISPLAY_CONFIRM_ACTION_KEEP,
    DISPLAY_CONFIRM_ACTION_ROLLBACK,
    DISPLAY_CONFIRM_ACTION_ROLLBACK_IN_PROGRESS
};

static volatile int display_confirm_pending = 0;
static volatile int display_confirm_visible = 0;
static volatile int display_confirm_selection = DISPLAY_CONFIRM_GO_BACK;
static volatile unsigned int display_confirm_seconds = 0;
static volatile int display_confirm_action = DISPLAY_CONFIRM_ACTION_NONE;

/*
 * H4C3D16:
 * True only while the main/render thread is inside the synchronous candidate
 * display reconstruction.  The controller thread already owns the independent
 * wall-clock confirmation countdown; this flag lets it distinguish an ordinary
 * confirmation timeout from a main-thread transition stall.
 */
static volatile int display_transition_switch_in_progress = 0;

static const ps2vnc_video_mode_t *display_confirm_previous_mode = NULL;
static unsigned int display_confirm_previous_x = 0;
static unsigned int display_confirm_previous_y = 0;
static unsigned int display_confirm_previous_w = 0;
static unsigned int display_confirm_previous_h = 0;
static unsigned int display_confirm_previous_desktop_w = 0;
static unsigned int display_confirm_previous_desktop_h = 0;
static ps2vnc_backend_t display_confirm_previous_backend =
    PS2VNC_BACKEND_NONE;

/*
 * D17AL-F3:
 * screen_fit_* is part of the complete local display profile.
 *
 * A provisional mode switch must not allow candidate B's calibration state
 * to survive a Go Back/timeout reconstruction of confirmed profile A.
 */
static int display_confirm_previous_screen_fit_width = 0;
static int display_confirm_previous_screen_fit_height = 0;
static int display_confirm_previous_screen_fit_offset_x = 0;
static int display_confirm_previous_screen_fit_offset_y = 0;
static int display_confirm_previous_screen_fit_full_bypass = 0;

static volatile u64 display_confirm_started_tick = 0;

/*
 * H4B2C:
 * Successful rollback is not the end of the global transaction.
 *
 * Once complete profile A has been reconstructed and the Pi has accepted
 * RESTORED, this acknowledgement owns local UI until the main thread sends
 * ACK successfully.  The controller never performs management HTTP.
 */
static volatile int display_restore_ack_visible = 0;
static volatile int display_restore_ack_request = 0;
static const ps2vnc_video_mode_t *display_restore_ack_mode = NULL;

static volatile int display_menu_parent_system = 0;

/*
 * TEST15E3-E:
 * Stable semantic runtime actions.  Button/chord assignment is still
 * compiled-in for this experiment, but controller bindings do not own the
 * implementation of these actions.  This is the boundary that a future
 * config-backed binding layer can target.
 */
typedef enum {
    RUNTIME_ACTION_NONE = 0,
    RUNTIME_ACTION_REFRESH_RFB,
    RUNTIME_ACTION_DISPLAY_SETTINGS,
    RUNTIME_ACTION_EXIT_TO_SYSTEM_MENU,
    RUNTIME_ACTION_SEND_TAB,
    RUNTIME_ACTION_SEND_ENTER
} ps2vnc_runtime_action_t;

enum {
    SYSTEM_MENU_REFRESH = 0,
    SYSTEM_MENU_DISPLAY,
    SYSTEM_MENU_EXIT,
    SYSTEM_MENU_ITEM_COUNT
};

enum {
    SYSTEM_MENU_STATUS_NONE = 0,
    SYSTEM_MENU_STATUS_REFRESHING,
    SYSTEM_MENU_STATUS_REFRESH_COMPLETE
};

#define SYSTEM_CHORD_HOLD_POLLS 120u
#define RUNTIME_HOTKEY_SETTLE_POLLS 8u
#define RUNTIME_HOTKEY_EXTENDABLE_SETTLE_POLLS 18u

#define RUNTIME_HOTKEY_BUTTON_MASK \
    (PAD_L1 | PAD_L2 | PAD_R1 | PAD_R2 | PAD_START)

enum {
    HOTKEY_TRIGGER_SETTLE = 1,
    HOTKEY_TRIGGER_RELEASE,
    HOTKEY_TRIGGER_HOLD
};

enum {
    HOTKEY_CONTEXT_DESKTOP = 1,
    HOTKEY_CONTEXT_GLOBAL
};

typedef struct {
    unsigned int mask;
    int action;
    int trigger;
    int context;
} ps2vnc_hotkey_binding_t;

static const ps2vnc_hotkey_binding_t runtime_hotkeys[] = {
    { PAD_R1, RUNTIME_ACTION_SEND_TAB,
      HOTKEY_TRIGGER_RELEASE, HOTKEY_CONTEXT_DESKTOP },

    { PAD_L1 | PAD_START, RUNTIME_ACTION_SEND_ENTER,
      HOTKEY_TRIGGER_RELEASE, HOTKEY_CONTEXT_DESKTOP },

    { PAD_L1 | PAD_R1, RUNTIME_ACTION_DISPLAY_SETTINGS,
      HOTKEY_TRIGGER_SETTLE, HOTKEY_CONTEXT_DESKTOP },

    { PAD_L1 | PAD_L2 | PAD_R1 | PAD_R2,
      RUNTIME_ACTION_REFRESH_RFB,
      HOTKEY_TRIGGER_HOLD, HOTKEY_CONTEXT_GLOBAL },

    { PAD_L1 | PAD_L2 | PAD_R1 | PAD_R2 | PAD_START,
      RUNTIME_ACTION_EXIT_TO_SYSTEM_MENU,
      HOTKEY_TRIGGER_HOLD, HOTKEY_CONTEXT_GLOBAL }
};

#define RUNTIME_HOTKEY_COUNT \
    (sizeof(runtime_hotkeys) / sizeof(runtime_hotkeys[0]))

static const ps2vnc_hotkey_binding_t *
runtime_hotkey_lookup(unsigned int mask, int desktop_context)
{
    unsigned int i;

    for (i = 0; i < RUNTIME_HOTKEY_COUNT; i++) {
        const ps2vnc_hotkey_binding_t *b = &runtime_hotkeys[i];

        if (b->mask != mask)
            continue;

        if (b->context == HOTKEY_CONTEXT_DESKTOP &&
            !desktop_context)
            continue;

        return b;
    }

    return NULL;
}

/*
 * A settle-triggered chord needs a little more human construction time when
 * its exact button mask can still grow into a larger hold-confirmed system
 * chord.  This is derived from the binding table rather than hard-coded
 * chord priority.
 *
 * Example:
 *
 *   L1+R1 is Display Settings, but it is also a strict subset of the
 *   four-shoulder Refresh chord.  Do not let Display win merely because
 *   the user's L2/R2 fingers arrived a fraction of a second later.
 *
 * A HOLD binding itself does not need this extra grace.  Once four shoulders
 * have resolved to pending Refresh, adding Start changes the exact mask and
 * naturally restarts collection for Exit.
 */
static int runtime_hotkey_has_hold_extension(
    unsigned int mask,
    int desktop_context)
{
    unsigned int i;

    for (i = 0; i < RUNTIME_HOTKEY_COUNT; i++) {
        const ps2vnc_hotkey_binding_t *b = &runtime_hotkeys[i];

        if (b->trigger != HOTKEY_TRIGGER_HOLD)
            continue;

        if (b->context == HOTKEY_CONTEXT_DESKTOP &&
            !desktop_context)
            continue;

        if (b->mask == mask)
            continue;

        if ((b->mask & mask) == mask)
            return 1;
    }

    return 0;
}

static volatile int system_menu_visible = 0;
static volatile int system_menu_selection = SYSTEM_MENU_REFRESH;
static volatile int system_menu_status = SYSTEM_MENU_STATUS_NONE;
static volatile int system_menu_countdown_action = RUNTIME_ACTION_NONE;
static volatile unsigned int system_menu_countdown_seconds = 0;
static volatile int runtime_action_request = RUNTIME_ACTION_NONE;

/*
 * D17AL-F8I1B:
 *
 * Refresh admission is system-level state, not menu-page state.
 * Navigation may change system_menu_status, but it must never bypass
 * REFRESHING/COOLDOWN ownership.
 */
enum {
    SYSTEM_REFRESH_READY = 0,
    SYSTEM_REFRESH_REFRESHING,
    SYSTEM_REFRESH_COOLDOWN
};

#define SYSTEM_REFRESH_COOLDOWN_SECONDS 3u

static volatile int system_refresh_state = SYSTEM_REFRESH_READY;
static volatile u64 system_refresh_cooldown_started_tick = 0;

/* Local GS-format keyboard texture buffer. */
static unsigned short osk_pixels[OSK_WIDTH * OSK_HEIGHT]
    __attribute__((aligned(128)));

static int screen_row = 0;

static void screenf(const char *fmt, ...)
{
    char buf[79];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    buf[sizeof(buf) - 1] = '\0';

    scr_setXY(0, screen_row++);
    scr_printf("%s", buf);
}

static void sanitize_text(char *s)
{
    int i;

    for (i = 0; s[i] != '\0'; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c < 32 || c > 126)
            s[i] = '.';
    }
}

static unsigned int read_be16(const unsigned char *p)
{
    return ((unsigned int)p[0] << 8) |
           ((unsigned int)p[1]);
}

static unsigned int read_be32(const unsigned char *p)
{
    return ((unsigned int)p[0] << 24) |
           ((unsigned int)p[1] << 16) |
           ((unsigned int)p[2] << 8)  |
           ((unsigned int)p[3]);
}


/*
 * TEST13J-D3-A4 geometry firewall.
 *
 * Return non-zero only when a non-empty rectangle is completely
 * contained within its owning coordinate space.
 *
 * Subtraction is deliberate. Do not validate with x + w or y + h,
 * because hostile/corrupted values could overflow that addition.
 */
static int region_fits_u32(
    unsigned int x,
    unsigned int y,
    unsigned int w,
    unsigned int h,
    unsigned int bounds_width,
    unsigned int bounds_height)
{
    if (w == 0 || h == 0)
        return 0;

    if (x >= bounds_width ||
        y >= bounds_height)
        return 0;

    if (w > bounds_width - x)
        return 0;

    if (h > bounds_height - y)
        return 0;

    return 1;
}


static int send_exact(int sock, const void *data, int len)
{
    const unsigned char *p = (const unsigned char *)data;
    int done = 0;

    while (done < len) {
        int n = send(sock, p + done, len - done, 0);

        if (n <= 0)
            return -1;

        done += n;
    }

    return done;
}


/*
 * Queue a controller-generated RFB message.
 *
 * Before the controller thread exists, fall back to a direct send.  After
 * the queue is active this function NEVER touches the VNC socket: it only
 * copies the complete RFB message into the producer/consumer ring.
 */
static int send_rfb_message(int sock, const void *data, int len)
{
    const unsigned char *src = (const unsigned char *)data;
    const struct timespec retry_delay = { 0, 1000000 }; /* 1 ms */

    if (rfb_queue_sema < 0)
        return send_exact(sock, data, len);

    if (len <= 0 || len > RFB_OUT_MESSAGE_MAX)
        return -1;

    /*
     * TEST12B:
     * Never allow mouse/key events generated during an outage to pile
     * up and fire into the replacement VNC session.
     */
    if (!rfb_connected)
        return len;

    for (;;) {
        unsigned int slot;

        if (WaitSema(rfb_queue_sema) < 0)
            return -1;

        if (rfb_out_count < RFB_OUT_QUEUE_CAPACITY) {
            slot = rfb_out_tail;

            rfb_out_queue[slot].len = (unsigned int)len;
            memcpy(rfb_out_queue[slot].data, src, len);

            rfb_out_tail =
                (rfb_out_tail + 1) % RFB_OUT_QUEUE_CAPACITY;
            rfb_out_count++;

            if (SignalSema(rfb_queue_sema) < 0)
                return -1;

            return len;
        }

        if (SignalSema(rfb_queue_sema) < 0)
            return -1;

        nanosleep(&retry_delay, NULL);
    }
}


/*
 * D17AL-F8J2-B4A:
 * Main-thread only. Discard controller-generated RFB messages after
 * the controller thread has acknowledged the hard libpad pause.
 *
 * This intentionally does NOT touch the RFB receive stream and does
 * NOT transmit any queued controller history into the transition.
 */
static int ps2vnc_discard_controller_rfb_queue(void)
{
    if (rfb_queue_sema < 0)
        return 0;

    if (WaitSema(rfb_queue_sema) < 0)
        return -1;

    rfb_out_head = rfb_out_tail;
    rfb_out_count = 0;

    if (SignalSema(rfb_queue_sema) < 0)
        return -1;

    return 0;
}


/*
 * Main-thread only: send every controller message currently queued.
 */
static int flush_rfb_outgoing(int sock)
{
    for (;;) {
        unsigned char msg[RFB_OUT_MESSAGE_MAX];
        unsigned int len;

        if (rfb_queue_sema < 0)
            return 0;

        if (WaitSema(rfb_queue_sema) < 0)
            return -1;

        if (rfb_out_count == 0) {
            if (SignalSema(rfb_queue_sema) < 0)
                return -1;
            return 0;
        }

        len = rfb_out_queue[rfb_out_head].len;

        if (len == 0 || len > RFB_OUT_MESSAGE_MAX) {
            SignalSema(rfb_queue_sema);
            return -1;
        }

        memcpy(msg, rfb_out_queue[rfb_out_head].data, len);

        rfb_out_head =
            (rfb_out_head + 1) % RFB_OUT_QUEUE_CAPACITY;
        rfb_out_count--;

        if (SignalSema(rfb_queue_sema) < 0)
            return -1;

        if (send_exact(sock, msg, (int)len) != (int)len)
            return -1;

    }
}

/*
 * TEST12B:
 * Throw away everything belonging to the old RFB byte stream.
 *
 * The controller thread may continue polling the pad, but rfb_connected
 * is cleared before this function is called, so new controller events
 * are discarded rather than entering this queue.
 */
static void reset_rfb_transport_state(void)
{
    rfb_rx_pos = 0;
    rfb_rx_end = 0;

    rfb_last_recv_n = 0;
    rfb_last_recv_call = 0;

    if (rfb_queue_sema >= 0) {
        if (WaitSema(rfb_queue_sema) >= 0) {
            rfb_out_head = 0;
            rfb_out_tail = 0;
            rfb_out_count = 0;
            SignalSema(rfb_queue_sema);
        }
    }
}



/*
 * Read exactly len bytes.
 *
 * Before the controller starts, the handshake/initial frame uses the old
 * blocking recv().
 *
 * After the controller starts, the main thread polls recv(MSG_DONTWAIT).
 * While waiting it drains queued controller events itself.  Therefore every
 * send()/recv() on the VNC socket is executed by THIS SAME EE thread.
 */
static void debug_emit_idle_poll(void);
static void debug_emit_now(void);

static int ps2vnc_remote_control_poll_idle(void);
static int ps2vnc_remote_display_lease_expired(void);

/*
 * TEST12B carrier-loss detection:
 * recv_exact() is defined before the Ethernet helper itself.
 */
static int ethGetNetIFLinkStatus(void);

/*
 * D17AL-F4:
 *
 * The confirmation UI must remain responsive while an incremental
 * FramebufferUpdateRequest is legitimately waiting for remote damage.
 *
 * RFB_RECEIVE_LOCAL_REDRAW is permitted only at a server-message
 * boundary, before recv_exact() has consumed any byte of that message.
 * It is NOT a socket/protocol error and must never interrupt a partially
 * consumed RFB structure.
 */
#define RFB_RECEIVE_LOCAL_REDRAW (-2)

static volatile int rfb_message_boundary_local_yield = 0;
static volatile int rfb_confirm_live_yield_enabled = 0;
static volatile unsigned int rfb_confirm_wait_generation = 0;

static int recv_exact(int sock, void *data, int len)
{
    unsigned char *dst = (unsigned char *)data;
    int done = 0;
    const struct timespec retry_delay = { 0, 1000000 }; /* 1 ms */

    while (done < len) {
        unsigned int available;
        unsigned int need;
        unsigned int take;

        if (exit_requested)
            return -1;

        if (display_confirm_pending &&

            display_confirm_action == DISPLAY_CONFIRM_ACTION_ROLLBACK)

            return -1;


        if (runtime_action_request == RUNTIME_ACTION_REFRESH_RFB) {
            debug_emit_now();
            return -1;
        }

        /*
         * D17AL-F4:
         * A benign local redraw has lower priority than exit, rollback,
         * and explicit RFB recovery. Yield only before this recv_exact()
         * has consumed anything and only at a complete server-message
         * boundary.
         */
        if (rfb_message_boundary_local_yield &&
            rfb_confirm_live_yield_enabled &&
            done == 0 &&
            osk_generation != rfb_confirm_wait_generation) {

            return RFB_RECEIVE_LOCAL_REDRAW;
        }

        /*
         * First satisfy the request from bytes already buffered in EE RAM.
         */
        available = rfb_rx_end - rfb_rx_pos;

        if (available > 0) {
            need = (unsigned int)(len - done);
            take = (available < need) ? available : need;

            memcpy(
                dst + done,
                rfb_rx_buffer + rfb_rx_pos,
                take
            );

            rfb_rx_pos += take;
            done += (int)take;

            if (rfb_rx_pos == rfb_rx_end) {
                rfb_rx_pos = 0;
                rfb_rx_end = 0;
            }

            continue;
        }

        /*
         * Buffer is empty.  Refill it with one comparatively large recv().
         *
         * Before the controller queue exists (handshake / initial setup),
         * use an ordinary blocking recv.  Afterwards use nonblocking recv so
         * the same main thread can continue draining controller events while
         * waiting for server data.
         */
        {
            u64 refill_start_tick = GetTimerSystemTime();

            for (;;) {
                int n;

            if (exit_requested)
                return -1;

            if (display_confirm_pending &&

                display_confirm_action == DISPLAY_CONFIRM_ACTION_ROLLBACK)

                return -1;


            if (runtime_action_request == RUNTIME_ACTION_REFRESH_RFB) {
                debug_emit_now();
                return -1;
            }

            /*
             * The refill loop can remain in EAGAIN indefinitely. Re-check
             * the benign message-boundary condition after all hard actions
             * on every idle poll.
             */
            if (rfb_message_boundary_local_yield &&
                rfb_confirm_live_yield_enabled &&
                done == 0 &&
                osk_generation != rfb_confirm_wait_generation) {

                return RFB_RECEIVE_LOCAL_REDRAW;
            }

            /*
             * TEST15B2G:
             * Do not prefill the 32 KiB receive buffer on every socket poll.
             * recv() supplies the authoritative byte count and rfb_rx_end
             * bounds all subsequent reads.  The old Test31 sentinel fill was
             * diagnostic instrumentation in a performance-critical path.
             */
            if (rfb_queue_sema >= 0) {
                if (flush_rfb_outgoing(sock) < 0)
                    return -1;

                n = recv(
                    sock,
                    rfb_rx_buffer,
                    sizeof(rfb_rx_buffer),
                    MSG_DONTWAIT
                );
            } else {
                n = recv(
                    sock,
                    rfb_rx_buffer,
                    sizeof(rfb_rx_buffer),
                    0
                );
            }

            rfb_rx_syscalls++;
            rfb_last_recv_n = n;
            rfb_last_recv_call = rfb_rx_syscalls;

            if (n > 0) {
                u64 elapsed = GetTimerSystemTime() - refill_start_tick;
                profile_rx_wait_ticks += elapsed;
                if (profile_hextile_active)
                    profile_hextile_rx_wait_ticks += elapsed;

                rfb_rx_pos = 0;
                rfb_rx_end = (unsigned int)n;
                break;
            }

            if (n == 0)
                return -1;

            if (rfb_queue_sema < 0)
                return -1;

            if (errno != EAGAIN && errno != EWOULDBLOCK)
                return -1;

            /*
             * TEST12B:
             * A physically lost Ethernet link may leave this TCP socket
             * returning EAGAIN indefinitely when there is no outgoing
             * controller traffic. Detect carrier loss proactively so an
             * idle/frozen desktop enters the normal reconnect supervisor.
             */
            if (!ethGetNetIFLinkStatus())
                return -1;

            if (ps2vnc_remote_display_lease_expired())
                return -1;

            if (rfb_message_boundary_local_yield &&
                done == 0 &&
                ps2vnc_remote_control_poll_idle() > 0) {

                return RFB_RECEIVE_LOCAL_REDRAW;
            }

            debug_emit_idle_poll();
            nanosleep(&retry_delay, NULL);
            }
        }
    }

    return done;
}


static const char *debug_stage_name(unsigned int stage)
{
    switch (stage) {
        case DBG_STAGE_STARTUP:          return "STARTUP";
        case DBG_STAGE_NET_READY:        return "NET_READY";
        case DBG_STAGE_GS_INIT:          return "GS_INIT";
        case DBG_STAGE_INITIAL_TEXTURE:  return "INITIAL_TEXTURE";
        case DBG_STAGE_CONTROLLER_START: return "CONTROLLER_START";
        case DBG_STAGE_LOOP_BEGIN:       return "LOOP_BEGIN";
        case DBG_STAGE_PRE_DRAW:         return "PRE_DRAW";
        case DBG_STAGE_POST_DRAW:        return "POST_DRAW";
        case DBG_STAGE_PRE_FLIP:         return "PRE_FLIP";
        case DBG_STAGE_POST_FLIP:        return "POST_FLIP";
        case DBG_STAGE_PRE_QUEUE:        return "PRE_QUEUE";
        case DBG_STAGE_POST_QUEUE:       return "POST_QUEUE";
        case DBG_STAGE_PRE_REQUEST:      return "PRE_REQUEST";
        case DBG_STAGE_POST_REQUEST:     return "POST_REQUEST";
        case DBG_STAGE_WAIT_RFB:         return "WAIT_RFB";
        case DBG_STAGE_POST_RECEIVE:     return "POST_RECEIVE";
        case DBG_STAGE_PRE_TEXTURE:      return "PRE_TEXTURE";
        case DBG_STAGE_POST_TEXTURE:     return "POST_TEXTURE";
        case DBG_STAGE_CONTROLLER_ERROR: return "CONTROLLER_ERROR";
        case DBG_STAGE_REQUEST_ERROR:    return "REQUEST_ERROR";
        case DBG_STAGE_RECEIVE_ERROR:    return "RECEIVE_ERROR";
        default:                         return "UNKNOWN";
    }
}

static int debug_udp_init(void)
{
    debug_udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (debug_udp_sock < 0)
        return -1;

    memset(&debug_udp_addr, 0, sizeof(debug_udp_addr));
    debug_udp_addr.sin_len = sizeof(debug_udp_addr);
    debug_udp_addr.sin_family = AF_INET;
    debug_udp_addr.sin_port = htons(DEBUG_UDP_PORT);
    debug_udp_addr.sin_addr.s_addr =
        inet_addr(PS2VNC_BOOTSTRAP_PI_IP);

    return 0;
}

static void debug_emit_now(void)
{
    char msg[640];
    int len;

    if (debug_udp_sock < 0)
        return;

    len = snprintf(
        msg, sizeof(msg),
        "DBG frame=%u stage=%s(%u) bytes=%d msg=%u "
        "rect=%u/%u xywh=%u,%u,%u,%u enc=%08x ctl=%u cerr=%d osk=%d "
        "hdr=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x "
        "hs=%u/%u hav=%u he=%u/%u rn=%d rc=%u "
        "hkm=%04x hks=%u/%u hka=%d hkt=%d hhp=%u hcd=%u "
        "hkl=%d hq=%04x sm=%d dm=%d rar=%d "
        "b4=%u ppr=%d ppa=%d q=%u",
        debug_frame_counter,
        debug_stage_name(debug_stage_id),
        debug_stage_id,
        debug_changed_bytes,
        live_message_type,
        live_rect_index,
        live_rect_count,
        live_rect_x,
        live_rect_y,
        live_rect_w,
        live_rect_h,
        live_encoding,
        controller_loop_counter,
        controller_thread_error,
        osk_visible,
        live_hdr_raw[0],
        live_hdr_raw[1],
        live_hdr_raw[2],
        live_hdr_raw[3],
        live_hdr_raw[4],
        live_hdr_raw[5],
        live_hdr_raw[6],
        live_hdr_raw[7],
        live_hdr_raw[8],
        live_hdr_raw[9],
        live_hdr_raw[10],
        live_hdr_raw[11],
        live_hdr_start_pos,
        live_hdr_start_end,
        live_hdr_start_avail,
        live_hdr_end_pos,
        live_hdr_end_end,
        live_hdr_recv_n,
        live_hdr_recv_call,
        debug_hotkey_mask,
        debug_hotkey_stable_polls,
        debug_hotkey_settle_target,
        debug_hotkey_action,
        debug_hotkey_trigger,
        debug_hotkey_hold_polls,
        system_menu_countdown_seconds,
        debug_hotkey_latched,
        debug_hotkey_quarantine,
        system_menu_visible,
        display_menu_visible,
        runtime_action_request,
        debug_transition_phase,
        controller_pad_pause_requested,
        controller_pad_pause_ack,
        rfb_out_count
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


static unsigned int profile_ticks_us(u64 ticks)
{
    u32 sec = 0;
    u32 usec = 0;

    TimerBusClock2USec(ticks, &sec, &usec);
    return sec * 1000000u + usec;
}


/*
 * D17AL-F8I1B:
 *
 * The System menu is global over ORDINARY local UI, but it must never
 * punch through a provisional display transaction or libpad calibration
 * ownership boundary.
 *
 * Pending mode/calibration requests are included so there is no race
 * between controller publication and main-thread transaction ownership.
 */
static int ps2vnc_system_overlay_allowed(void)
{
    if (display_restore_ack_visible ||
        display_confirm_pending ||
        display_transition_switch_in_progress ||
        display_mode_request != DISPLAY_MODE_REQUEST_NONE ||
        display_calibration_request != DISPLAY_MODE_REQUEST_NONE ||
        controller_pad_pause_requested)
        return 0;

    return 1;
}


/*
 * Return nonzero only when COOLDOWN actually expires.
 */
static int ps2vnc_system_refresh_update_state(void)
{
    unsigned int elapsed_us;

    if (system_refresh_state != SYSTEM_REFRESH_COOLDOWN)
        return 0;

    elapsed_us =
        profile_ticks_us(
            GetTimerSystemTime() -
            system_refresh_cooldown_started_tick);

    if (elapsed_us <
        SYSTEM_REFRESH_COOLDOWN_SECONDS * 1000000u)
        return 0;

    system_refresh_state = SYSTEM_REFRESH_READY;
    system_refresh_cooldown_started_tick = 0;
    system_menu_status = SYSTEM_MENU_STATUS_NONE;

    if (system_menu_visible)
        osk_generation++;

    return 1;
}


/*
 * Sole publication gate for an explicit manual Refresh request.
 */
static int ps2vnc_system_refresh_accept_request(void)
{
    (void)ps2vnc_system_refresh_update_state();

    if (!ps2vnc_system_overlay_allowed())
        return 0;

    if (system_refresh_state != SYSTEM_REFRESH_READY)
        return 0;

    if (runtime_action_request != RUNTIME_ACTION_NONE)
        return 0;

    system_refresh_state = SYSTEM_REFRESH_REFRESHING;
    system_refresh_cooldown_started_tick = 0;

    system_menu_status = SYSTEM_MENU_STATUS_REFRESHING;
    system_menu_selection = SYSTEM_MENU_REFRESH;

    runtime_action_request = RUNTIME_ACTION_REFRESH_RFB;

    osk_generation++;
    return 1;
}


/*
 * Exit remains available during ordinary Refresh/recovery, but never cuts
 * across a provisional display transaction.
 */
static int ps2vnc_system_exit_accept_request(void)
{
    if (!ps2vnc_system_overlay_allowed())
        return 0;

    runtime_action_request =
        RUNTIME_ACTION_EXIT_TO_SYSTEM_MENU;

    exit_requested = 1;
    return 1;
}


static void display_confirm_arm_transaction(
    const ps2vnc_video_mode_t *previous_mode,
    unsigned int previous_x,
    unsigned int previous_y,
    unsigned int previous_w,
    unsigned int previous_h,
    int previous_screen_fit_width,
    int previous_screen_fit_height,
    int previous_screen_fit_offset_x,
    int previous_screen_fit_offset_y,
    int previous_screen_fit_full_bypass)
{
    display_confirm_previous_mode = previous_mode;
    display_confirm_previous_backend = active_display_backend;
    display_confirm_previous_x = previous_x;
    display_confirm_previous_y = previous_y;
    display_confirm_previous_w = previous_w;
    display_confirm_previous_h = previous_h;
    display_confirm_previous_desktop_w = desktop_width;
    display_confirm_previous_desktop_h = desktop_height;

    display_confirm_previous_screen_fit_width =
        previous_screen_fit_width;
    display_confirm_previous_screen_fit_height =
        previous_screen_fit_height;
    display_confirm_previous_screen_fit_offset_x =
        previous_screen_fit_offset_x;
    display_confirm_previous_screen_fit_offset_y =
        previous_screen_fit_offset_y;
    display_confirm_previous_screen_fit_full_bypass =
        previous_screen_fit_full_bypass;

    display_confirm_selection = DISPLAY_CONFIRM_GO_BACK;
    display_confirm_seconds = DISPLAY_CONFIRM_TIMEOUT_SECONDS;
    display_confirm_action = DISPLAY_CONFIRM_ACTION_NONE;
    display_confirm_visible = 0;
    display_confirm_started_tick = GetTimerSystemTime();

    /* Publish pending last: controller may observe it immediately. */
    display_confirm_pending = 1;
}


static void display_confirm_disarm_transaction(void)
{
    /* Withdraw ownership first; remaining fields are then private cleanup. */
    display_confirm_pending = 0;
    display_confirm_visible = 0;
    display_confirm_seconds = 0;
    display_confirm_action = DISPLAY_CONFIRM_ACTION_NONE;
    display_confirm_previous_mode = NULL;
    display_confirm_previous_backend = PS2VNC_BACKEND_NONE;
    display_confirm_previous_x = 0;
    display_confirm_previous_y = 0;
    display_confirm_previous_w = 0;
    display_confirm_previous_h = 0;
    display_confirm_previous_desktop_w = 0;
    display_confirm_previous_desktop_h = 0;

    display_confirm_previous_screen_fit_width = 0;
    display_confirm_previous_screen_fit_height = 0;
    display_confirm_previous_screen_fit_offset_x = 0;
    display_confirm_previous_screen_fit_offset_y = 0;
    display_confirm_previous_screen_fit_full_bypass = 0;

    display_confirm_started_tick = 0;
}



static void display_confirm_return_to_active_menu(void)
{
    /*
     * F8J1C:
     * Remote test restoration is automatically acknowledged.  Do not
     * synthesize a Display Modes UI that was never open before the test.
     *
     * All ordinary human KEEP/Go Back/recovery transactions retain their
     * historical return-to-active-row behavior.
     */
    if (remote_display_suppress_next_active_menu_return) {
        remote_display_suppress_next_active_menu_return = 0;
        return;
    }

    /*
     * H4B2C:
     * Never expose Display Settings until durable restore ACK ownership
     * has completed.
     */
    if (display_restore_ack_visible)
        return;

    display_menu_open_for_active(0);
}



static unsigned int display_confirm_remaining_seconds(void)
{
    unsigned int elapsed_us;

    if (!display_confirm_pending)
        return 0;

    elapsed_us = profile_ticks_us(
        GetTimerSystemTime() - display_confirm_started_tick);

    if (elapsed_us >=
        DISPLAY_CONFIRM_TIMEOUT_SECONDS * 1000000u)
        return 0;

    return DISPLAY_CONFIRM_TIMEOUT_SECONDS -
        (elapsed_us / 1000000u);
}

static void profile_emit_now(void)
{
    char msg[256];
    int len;

    if (debug_udp_sock >= 0) {
        len = snprintf(
            msg, sizeof(msg),
            "PRF frame=%u rn=%u dn=%u bytes=%u raw=%u hx=%u "
            "ps=%u fl=%u rfb=%u rx=%u hd=%u hr=%u dw=%u cp=%u",
            debug_frame_counter,
            profile_rfb_updates,
            profile_dirty_updates,
            profile_changed_bytes,
            profile_raw_rects,
            profile_hextile_rects,
            profile_ticks_us(profile_present_sync_ticks),
            profile_ticks_us(profile_flip_ticks),
            profile_ticks_us(profile_rfb_ticks),
            profile_ticks_us(profile_rx_wait_ticks),
            profile_ticks_us(profile_hextile_ticks),
            profile_ticks_us(profile_hextile_rx_wait_ticks),
            profile_ticks_us(profile_dirty_wait_ticks),
            profile_ticks_us(profile_dirty_work_ticks)
        );

        if (len > 0)
            sendto(debug_udp_sock, msg, len, 0,
                   (struct sockaddr *)&debug_udp_addr,
                   sizeof(debug_udp_addr));
    }

    profile_present_sync_ticks = 0;
    profile_flip_ticks = 0;
    profile_rfb_ticks = 0;
    profile_rx_wait_ticks = 0;
    profile_hextile_ticks = 0;
    profile_hextile_rx_wait_ticks = 0;
    profile_dirty_wait_ticks = 0;
    profile_dirty_work_ticks = 0;
    profile_rfb_updates = 0;
    profile_dirty_updates = 0;
    profile_changed_bytes = 0;
    profile_raw_rects = 0;
    profile_hextile_rects = 0;
}


static void debug_emit_idle_poll(void)
{
    debug_idle_poll_counter++;

    if ((debug_idle_poll_counter % 250) == 0)
        debug_emit_now();
}

/*
 * Wait for libpad to finish an asynchronous mode change.
 *
 * The PS2SDK pad sample waits for STABLE/FINDCTP1 after
 * padSetMainMode().  Give this controller up to ~2 seconds.
 */
static int wait_pad_ready_after_mode_change(void)
{
    const struct timespec delay = { 0, 10000000 }; /* 10 ms */
    int i;

    for (i = 0; i < 200; i++) {
        int state = padGetState(0, 0);

        if (state == PAD_STATE_STABLE ||
            state == PAD_STATE_FINDCTP1)
            return 0;

        nanosleep(&delay, NULL);
    }

    return -1;
}


/*
 * Startup-only display recalibration chord.
 *
 * The normal libpad button bits are active-low, matching the controller
 * thread's existing 0xFFFF ^ buttons.btns decoding.
 *
 * Normal startup returns immediately when L1+R1 are not held.  If both
 * buttons are already down, require about 300 ms of continuous hold before
 * accepting the override.
 */
static int startup_force_calibration_requested(void)
{
    const struct timespec delay = { 0, 10000000 }; /* 10 ms */
    struct padButtonStatus buttons;
    unsigned int pressed;
    int consecutive = 0;
    int saw_valid_sample = 0;
    int i;

    for (i = 0; i < 50; i++) {

        if (padGetState(0, 0) == PAD_STATE_STABLE &&
            padRead(0, 0, &buttons) != 0) {

            pressed = 0xFFFFu ^ (unsigned int)buttons.btns;

            if (!saw_valid_sample) {
                saw_valid_sample = 1;

                if ((pressed & (PAD_L1 | PAD_R1)) !=
                    (PAD_L1 | PAD_R1))
                    return 0;
            }

            if ((pressed & (PAD_L1 | PAD_R1)) ==
                (PAD_L1 | PAD_R1)) {

                consecutive++;

                if (consecutive >= 30)
                    return 1;

            } else {
                return 0;
            }
        }

        nanosleep(&delay, NULL);
    }

    return 0;
}


static void EthStatusCheckCb(s32 alarm_id, u16 time, void *common)
{
    iWakeupThread(*(int *)common);
}

static int WaitValidNetState(int (*checkingFunction)(void))
{
    int thread_id;
    int retry_cycles;

    thread_id = GetThreadId();

    for (retry_cycles = 0; checkingFunction() == 0; retry_cycles++) {
        SetAlarm(1000 * 16, &EthStatusCheckCb, &thread_id);
        SleepThread();

        if (retry_cycles >= 9)
            return -1;
    }

    return 0;
}

static int ethGetNetIFLinkStatus(void)
{
    return NetManIoctl(
        NETMAN_NETIF_IOCTL_GET_LINK_STATUS,
        NULL, 0, NULL, 0) == NETMAN_NETIF_ETH_LINK_STATE_UP;
}

static int ethWaitValidNetIFLinkState(void)
{
    return WaitValidNetState(&ethGetNetIFLinkStatus);
}

static int rfb_connect_and_handshake_limited(
    unsigned int max_width,
    unsigned int max_height)
{
    static const char client_version[12] = "RFB 003.008\n";

    int sock = -1;
    int i;
    int none_found = 0;
    struct sockaddr_in server;

    char server_version[13];
    unsigned char security_count;
    unsigned char security_types[255];
    unsigned char security_choice = 1;
    unsigned char security_result[4];
    unsigned char shared_flag = 1;
    unsigned char server_init[24];

    unsigned int width;
    unsigned int height;
    unsigned int name_len;
    unsigned int name_take;
    unsigned int remaining;

    char desktop_name[128];
    unsigned char discard[64];

    screenf(
        "Connecting to %s:%d...",
        PS2VNC_BOOTSTRAP_PI_IP,
        VNC_PORT
    );

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        screenf("ERROR: socket() failed: %d", sock);
        goto fail;
    }

    memset(&server, 0, sizeof(server));
    server.sin_len = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_port = htons(VNC_PORT);
    server.sin_addr.s_addr =
        inet_addr(PS2VNC_BOOTSTRAP_PI_IP);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        screenf("ERROR: connect() failed");
        goto fail;
    }

    screenf("TCP connected");

    if (recv_exact(sock, server_version, 12) != 12) {
        screenf("ERROR: could not read RFB version");
        goto fail;
    }

    server_version[12] = '\0';

    if (server_version[11] == '\n')
        server_version[11] = '\0';

    sanitize_text(server_version);
    screenf("Server version: %s", server_version);

    if (send_exact(sock, client_version, 12) != 12) {
        screenf("ERROR: could not send RFB version");
        goto fail;
    }

    if (recv_exact(sock, &security_count, 1) != 1) {
        screenf("ERROR: no security-type count");
        goto fail;
    }

    if (security_count == 0) {
        unsigned char lenbuf[4];
        unsigned int reason_len;
        unsigned int take;
        char reason[128];

        if (recv_exact(sock, lenbuf, 4) != 4) {
            screenf("ERROR: server rejected connection");
            goto fail;
        }

        reason_len = read_be32(lenbuf);
        take = reason_len;

        if (take > sizeof(reason) - 1)
            take = sizeof(reason) - 1;

        if (take > 0 && recv_exact(sock, reason, take) != (int)take) {
            screenf("ERROR: failed reading rejection");
            goto fail;
        }

        reason[take] = '\0';
        sanitize_text(reason);

        remaining = reason_len - take;

        while (remaining > 0) {
            unsigned int chunk = remaining;

            if (chunk > sizeof(discard))
                chunk = sizeof(discard);

            if (recv_exact(sock, discard, chunk) != (int)chunk)
                break;

            remaining -= chunk;
        }

        screenf("SERVER REJECTED: %s", reason);
        goto fail;
    }

    if (recv_exact(sock, security_types, security_count) != security_count) {
        screenf("ERROR: failed reading security types");
        goto fail;
    }

    for (i = 0; i < security_count; i++) {
        if (security_types[i] == 1) {
            none_found = 1;
            break;
        }
    }

    if (!none_found) {
        screenf("ERROR: SecurityType None not offered");
        goto fail;
    }

    if (send_exact(sock, &security_choice, 1) != 1) {
        screenf("ERROR: could not select security type");
        goto fail;
    }

    if (recv_exact(sock, security_result, 4) != 4) {
        screenf("ERROR: no SecurityResult");
        goto fail;
    }

    if (read_be32(security_result) != 0) {
        screenf("ERROR: authentication failed");
        goto fail;
    }

    if (send_exact(sock, &shared_flag, 1) != 1) {
        screenf("ERROR: could not send ClientInit");
        goto fail;
    }

    if (recv_exact(sock, server_init, sizeof(server_init)) != sizeof(server_init)) {
        screenf("ERROR: failed reading ServerInit");
        goto fail;
    }

    width = read_be16(&server_init[0]);
    height = read_be16(&server_init[2]);
    name_len = read_be32(&server_init[20]);

    screenf("Handshake OK - desktop %ux%u", width, height);

    /*
     * H4C3C2A:
     * The caller explicitly owns the ServerInit acceptance envelope.
     *
     * Ordinary callers still use OUTPUT_WIDTH/OUTPUT_HEIGHT through the
     * wrapper below. Startup reconciliation will later be allowed to use
     * the larger allocated RFB framebuffer capacity long enough to repair
     * a stale TigerVNC geometry.
     *
     * Never permit an envelope larger than PS2VNC's backing framebuffer.
     */
    if (max_width == 0 ||
        max_height == 0 ||
        max_width > VNC_WIDTH ||
        max_height > VNC_HEIGHT) {

        screenf(
            "ERROR: invalid RFB limit %ux%u",
            max_width,
            max_height
        );
        goto fail;
    }

    if (width == 0 || height == 0 ||
        width > max_width ||
        height > max_height) {

        screenf(
            "ERROR: unsupported VNC desktop %ux%u",
            width,
            height
        );
        goto fail;
    }

    if (desktop_geometry_locked) {
        if (width != desktop_width ||
            height != desktop_height) {

            screenf(
                "ERROR: expected %ux%u VNC desktop",
                desktop_width,
                desktop_height
            );
            goto fail;
        }
    } else {
        /*
         * A prior interrupted resize may have left TigerVNC smaller than
         * 1280x720. Treat the server's actual geometry as authoritative
         * until calibration locks the final safe desktop.
         */
        desktop_width = width;
        desktop_height = height;
    }

    name_take = name_len;

    if (name_take > sizeof(desktop_name) - 1)
        name_take = sizeof(desktop_name) - 1;

    if (name_take > 0) {
        if (recv_exact(sock, desktop_name, name_take) != (int)name_take) {
            screenf("ERROR: failed reading desktop name");
            goto fail;
        }
    }

    desktop_name[name_take] = '\0';
    sanitize_text(desktop_name);

    remaining = name_len - name_take;

    while (remaining > 0) {
        unsigned int chunk = remaining;

        if (chunk > sizeof(discard))
            chunk = sizeof(discard);

        if (recv_exact(sock, discard, chunk) != (int)chunk) {
            screenf("ERROR: failed discarding long name");
            goto fail;
        }

        remaining -= chunk;
    }

    screenf("Desktop name: %.55s", desktop_name);

    return sock;

fail:
    if (sock >= 0)
        close(sock);

    return -1;
}


/*
 * Preserve the historical handshake contract for every ordinary caller.
 *
 * H4C reconciliation must opt into a wider acceptance envelope explicitly;
 * no existing startup/recovery path is silently weakened.
 */
static int rfb_connect_and_handshake(void)
{
    return rfb_connect_and_handshake_limited(
        OUTPUT_WIDTH,
        OUTPUT_HEIGHT);
}


static int set_gs555_pixel_format(int sock)
{
    static const unsigned char msg[20] = {
        0, 0, 0, 0,
        16, 15, 0, 1,
        0, 31,
        0, 31,
        0, 31,
        0, 5, 10,
        0, 0, 0
    };

    return send_exact(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}

#define RFB_ENCODING_RAW                    0x00000000u
#define RFB_ENCODING_HEXTILE                0x00000005u
#define RFB_ENCODING_EXTENDED_DESKTOP_SIZE  0xFFFFFECCu

#define HEXTILE_RAW                  0x01u
#define HEXTILE_BACKGROUND_SPECIFIED 0x02u
#define HEXTILE_FOREGROUND_SPECIFIED 0x04u
#define HEXTILE_ANY_SUBRECTS         0x08u
#define HEXTILE_SUBRECTS_COLORED     0x10u

/*
 * TEST13J-D1:
 * State reported by the server's ExtendedDesktopSize pseudo-rectangle.
 *
 * D1 only proves capability and records the current layout. It does not
 * request a new size yet.
 */
static int rfb_extended_desktop_seen = 0;
static unsigned int rfb_extended_desktop_reason = 0;
static unsigned int rfb_extended_desktop_result = 0;
static unsigned int rfb_extended_desktop_width = 0;
static unsigned int rfb_extended_desktop_height = 0;
static unsigned int rfb_extended_desktop_screen_count = 0;

static unsigned int rfb_first_screen_id = 0;
static unsigned int rfb_first_screen_x = 0;
static unsigned int rfb_first_screen_y = 0;
static unsigned int rfb_first_screen_width = 0;
static unsigned int rfb_first_screen_height = 0;
static unsigned int rfb_first_screen_flags = 0;

/*
 * TEST13J-D2:
 * During a deliberate SetDesktopSize transaction, permit the EDS
 * response to report the requested dimensions even though the Raw
 * framebuffer path is still fixed at 1280x720.
 */
static int rfb_resize_expected = 0;
static unsigned int rfb_resize_expected_width = 0;
static unsigned int rfb_resize_expected_height = 0;


static int set_raw_encoding(int sock)
{
    static const unsigned char msg[8] = {
        2, 0,
        0, 1,
        0, 0, 0, 0
    };

    return send_exact(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}



/*
 * TEST15D1:
 * Prefer Hextile for ordinary incremental updates while retaining Raw
 * as the mandatory fallback. Startup, resize and recovery full-frame
 * synchronization remain Raw until explicitly switched to this list.
 */
static int set_live_encoding(int sock)
{
    static const unsigned char msg[12] = {
        2, 0,
        0, 2,

        /* Hextile = 5 */
        0x00, 0x00, 0x00, 0x05,

        /* Raw = 0 */
        0x00, 0x00, 0x00, 0x00
    };

    return send_exact(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}


/*
 * Advertise Raw plus ExtendedDesktopSize (-308).
 *
 * Raw remains first because it is still our preferred pixel encoding.
 * ExtendedDesktopSize is only a capability declaration.
 */
static int set_raw_extended_desktop_encoding(int sock)
{
    static const unsigned char msg[12] = {
        2, 0,
        0, 2,

        /* Raw = 0 */
        0x00, 0x00, 0x00, 0x00,

        /* ExtendedDesktopSize = -308 = 0xFFFFFECC */
        0xFF, 0xFF, 0xFE, 0xCC
    };

    return send_exact(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}



/*
 * Forward declaration: TEST13J-D2 resize confirmation needs to consume
 * an ExtendedDesktopSize FramebufferUpdate before the receiver's full
 * implementation later in this file.
 */
static int receive_framebuffer_update(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    int verbose,
    int require_full);


/*
 * TEST13J-D2:
 * Send RFB SetDesktopSize (client message type 251).
 *
 * This client uses a single-screen layout. The screen ID and flags are
 * preserved from the server's ExtendedDesktopSize capability response.
 */
static int send_set_desktop_size_and_probe(
    int sock,
    unsigned int width,
    unsigned int height)
{
    unsigned char msg[34];

    memset(msg, 0, sizeof(msg));

    /*
     * Bytes 0..23:
     * RFB SetDesktopSize.
     */
    msg[0] = 251;

    msg[2] = (unsigned char)((width >> 8) & 0xFF);
    msg[3] = (unsigned char)(width & 0xFF);

    msg[4] = (unsigned char)((height >> 8) & 0xFF);
    msg[5] = (unsigned char)(height & 0xFF);

    msg[6] = 1; /* one screen */

    msg[8]  =
        (unsigned char)((rfb_first_screen_id >> 24) & 0xFF);
    msg[9]  =
        (unsigned char)((rfb_first_screen_id >> 16) & 0xFF);
    msg[10] =
        (unsigned char)((rfb_first_screen_id >> 8) & 0xFF);
    msg[11] =
        (unsigned char)(rfb_first_screen_id & 0xFF);

    /* Screen x/y remain 0,0. */

    msg[16] = (unsigned char)((width >> 8) & 0xFF);
    msg[17] = (unsigned char)(width & 0xFF);

    msg[18] = (unsigned char)((height >> 8) & 0xFF);
    msg[19] = (unsigned char)(height & 0xFF);

    msg[20] =
        (unsigned char)((rfb_first_screen_flags >> 24) & 0xFF);
    msg[21] =
        (unsigned char)((rfb_first_screen_flags >> 16) & 0xFF);
    msg[22] =
        (unsigned char)((rfb_first_screen_flags >> 8) & 0xFF);
    msg[23] =
        (unsigned char)(rfb_first_screen_flags & 0xFF);

    /*
     * TEST13J-D2 SINGLE-WRITE PROBE:
     *
     * Bytes 24..33 are a second RFB client message:
     * an empty incremental FramebufferUpdateRequest.
     *
     * TigerVNC therefore receives:
     *
     *   SetDesktopSize
     *   FramebufferUpdateRequest
     *
     * from one PS2 send sequence.
     */
    msg[24] = 3; /* FramebufferUpdateRequest */
    msg[25] = 1; /* incremental */

    /*
     * TEST13J-D2:
     * Request one harmless pixel rather than an empty region.
     *
     * The RFB specification permits an empty request, but this avoids
     * depending on the server waking its update machinery for 0x0.
     */
    msg[26] = 0; /* x = 0 */
    msg[27] = 0;
    msg[28] = 0; /* y = 0 */
    msg[29] = 0;
    msg[30] = 0; /* width = 1 */
    msg[31] = 1;
    msg[32] = 0; /* height = 1 */
    msg[33] = 1;

    return send_exact(sock, msg, sizeof(msg)) ==
           sizeof(msg) ? 0 : -1;
}


static int request_and_confirm_desktop_size(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    unsigned int width,
    unsigned int height)
{
    unsigned int response_count;

    /*
     * F8J2-B2:
     *
     * SetDesktopSize completion is not necessarily the first EDS
     * notification TigerVNC emits at the new geometry.
     *
     * Hardware packet evidence showed:
     *
     *   reason=0 result=0 requested-size
     *   reason=0 result=0 requested-size
     *   reason=0 result=0 requested-size
     *   reason=1 result=0 requested-size
     *
     * reason=0 is an informational server-side layout notification.
     * reason=1 is the acknowledgement for THIS client's SetDesktopSize.
     *
     * Keep consuming complete FramebufferUpdate messages until the
     * matching client acknowledgement arrives.  Geometry validation
     * remains owned by receive_framebuffer_update() while
     * rfb_resize_expected is set.
     *
     * Bound the number of complete server updates accepted while waiting.
     * recv_exact() retains the existing exit/rollback/recovery escape
     * behavior inside each message.
     */
    const unsigned int response_limit = 16;

    rfb_resize_expected = 1;
    rfb_resize_expected_width = width;
    rfb_resize_expected_height = height;

    if (send_set_desktop_size_and_probe(
            sock,
            width,
            height) < 0) {

        rfb_resize_expected = 0;

        /* F8J2-B2 diagnostic: SetDesktopSize/probe send failure. */
        live_error_code = 120;

        return -1;
    }

    for (response_count = 0;
         response_count < response_limit;
         response_count++) {

        /*
         * Each receive consumes one complete FramebufferUpdate.
         * Clear the observation fields so an unrelated update cannot
         * inherit a previous EDS notification.
         */
        rfb_extended_desktop_seen = 0;
        rfb_extended_desktop_reason = 0;
        rfb_extended_desktop_result = 0;
        rfb_extended_desktop_width = 0;
        rfb_extended_desktop_height = 0;
        rfb_extended_desktop_screen_count = 0;

        if (receive_framebuffer_update(
                sock,
                rfb_framebuffer,
                gs_framebuffer,
                0,
                0) < 0) {

            rfb_resize_expected = 0;
            return -1;
        }

        /*
         * Ordinary framebuffer traffic may be interleaved while the
         * server completes the resize transaction.  It is not the
         * acknowledgement we are waiting for.
         */
        if (!rfb_extended_desktop_seen)
            continue;

        /*
         * receive_framebuffer_update() already rejects an EDS rectangle
         * whose dimensions differ from rfb_resize_expected_width/height.
         *
         * A same-geometry reason other than 1 is informational.  Continue
         * until the acknowledgement specifically associated with this
         * client's SetDesktopSize arrives.
         */
        if (rfb_extended_desktop_reason != 1)
            continue;

        rfb_resize_expected = 0;

        /*
         * reason=1 identifies our request.  result=0 is success.
         */
        if (rfb_extended_desktop_result != 0) {
            live_error_code = 121;
            return -1;
        }

        if (rfb_extended_desktop_width != width ||
            rfb_extended_desktop_height != height) {

            /*
             * Normally unreachable because the receiver validates this
             * while rfb_resize_expected is active.  Keep an explicit
             * defensive diagnostic at the ownership boundary.
             */
            live_error_code = 118;
            return -1;
        }

        return 0;
    }

    rfb_resize_expected = 0;

    /*
     * We consumed bounded complete server updates without seeing the
     * acknowledgement for our SetDesktopSize.
     */
    live_error_code = 122;

    return -1;
}

static int send_pointer_event(
    int sock,
    unsigned char button_mask,
    unsigned int x,
    unsigned int y)
{
    unsigned char msg[6];

    msg[0] = 5;
    msg[1] = button_mask;
    msg[2] = (unsigned char)((x >> 8) & 0xFF);
    msg[3] = (unsigned char)(x & 0xFF);
    msg[4] = (unsigned char)((y >> 8) & 0xFF);
    msg[5] = (unsigned char)(y & 0xFF);

    return send_rfb_message(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}


/*
 * RFB KeyEvent:
 *   byte 0     = message type 4
 *   byte 1     = down flag (1 press, 0 release)
 *   bytes 2-3  = padding
 *   bytes 4-7  = X11 keysym, big-endian
 *
 * This uses the same serialized sender as PointerEvent and
 * FramebufferUpdateRequest, so keyboard traffic cannot interleave with
 * either of them on the TCP byte stream.
 */
static int send_key_event(
    int sock,
    unsigned char down,
    unsigned int keysym)
{
    unsigned char msg[8];

    msg[0] = 4;
    msg[1] = down ? 1 : 0;
    msg[2] = 0;
    msg[3] = 0;
    msg[4] = (unsigned char)((keysym >> 24) & 0xFF);
    msg[5] = (unsigned char)((keysym >> 16) & 0xFF);
    msg[6] = (unsigned char)((keysym >> 8) & 0xFF);
    msg[7] = (unsigned char)(keysym & 0xFF);

    return send_rfb_message(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}


static int send_key_tap(int sock, unsigned int keysym)
{
    unsigned char msg[16];

    /* Key down. */
    msg[0] = 4;
    msg[1] = 1;
    msg[2] = 0;
    msg[3] = 0;
    msg[4] = (unsigned char)((keysym >> 24) & 0xFF);
    msg[5] = (unsigned char)((keysym >> 16) & 0xFF);
    msg[6] = (unsigned char)((keysym >> 8) & 0xFF);
    msg[7] = (unsigned char)(keysym & 0xFF);

    /* Key up. */
    msg[8]  = 4;
    msg[9]  = 0;
    msg[10] = 0;
    msg[11] = 0;
    msg[12] = msg[4];
    msg[13] = msg[5];
    msg[14] = msg[6];
    msg[15] = msg[7];

    return send_rfb_message(sock, msg, sizeof(msg)) == sizeof(msg) ? 0 : -1;
}


/*
 * Send a key with optional real Ctrl/Alt modifiers as one serialized RFB
 * write.  This keeps the entire shortcut atomic relative to framebuffer
 * requests and mouse events.
 *
 * Shift remains a local OSK layer; it changes the final printable keysym.
 */
static int send_modified_key_tap(
    int sock,
    unsigned int keysym,
    int use_ctrl,
    int use_alt)
{
    unsigned char msg[48];
    int pos = 0;

#define APPEND_KEY_EVENT(DOWN, KEYSYM) do {                       \
        unsigned int _k = (unsigned int)(KEYSYM);                 \
        msg[pos + 0] = 4;                                         \
        msg[pos + 1] = (DOWN) ? 1 : 0;                            \
        msg[pos + 2] = 0;                                         \
        msg[pos + 3] = 0;                                         \
        msg[pos + 4] = (unsigned char)((_k >> 24) & 0xFF);        \
        msg[pos + 5] = (unsigned char)((_k >> 16) & 0xFF);        \
        msg[pos + 6] = (unsigned char)((_k >> 8) & 0xFF);         \
        msg[pos + 7] = (unsigned char)(_k & 0xFF);                \
        pos += 8;                                                  \
    } while (0)

    if (use_ctrl)
        APPEND_KEY_EVENT(1, XK_CONTROL_L);

    if (use_alt)
        APPEND_KEY_EVENT(1, XK_ALT_L);

    APPEND_KEY_EVENT(1, keysym);
    APPEND_KEY_EVENT(0, keysym);

    if (use_alt)
        APPEND_KEY_EVENT(0, XK_ALT_L);

    if (use_ctrl)
        APPEND_KEY_EVENT(0, XK_CONTROL_L);

#undef APPEND_KEY_EVENT

    return send_rfb_message(sock, msg, pos) == pos ? 0 : -1;
}

static void osk_clear_one_shot_modifiers(void)
{
    if (osk_shift || osk_ctrl || osk_alt) {
        osk_shift = 0;
        osk_ctrl = 0;
        osk_alt = 0;
        osk_generation++;
    }
}

typedef struct {
    char c;
    unsigned char row[7];
} osk_glyph_t;

static const osk_glyph_t osk_font[] = {
    { ' ', { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { 'A', { 0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 } },
    { 'B', { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E } },
    { 'C', { 0x0F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0F } },
    { 'D', { 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E } },
    { 'E', { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F } },
    { 'F', { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10 } },
    { 'G', { 0x0F, 0x10, 0x10, 0x17, 0x11, 0x11, 0x0F } },
    { 'H', { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 } },
    { 'I', { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F } },
    { 'J', { 0x07, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0C } },
    { 'K', { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 } },
    { 'L', { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F } },
    { 'M', { 0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11 } },
    { 'N', { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 } },
    { 'O', { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E } },
    { 'P', { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 } },
    { 'Q', { 0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D } },
    { 'R', { 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11 } },
    { 'S', { 0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E } },
    { 'T', { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 } },
    { 'U', { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E } },
    { 'V', { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04 } },
    { 'W', { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A } },
    { 'X', { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11 } },
    { 'Y', { 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04 } },
    { 'Z', { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F } },
    { '0', { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E } },
    { '1', { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E } },
    { '2', { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F } },
    { '3', { 0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E } },
    { '4', { 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02 } },
    { '5', { 0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E } },
    { '6', { 0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E } },
    { '7', { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 } },
    { '8', { 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E } },
    { '9', { 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E } },
    { '-', { 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00 } },
    { '=', { 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00 } },
    { ';', { 0x00, 0x04, 0x00, 0x04, 0x04, 0x08, 0x00 } },
    { ',', { 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08 } },
    { '.', { 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06 } },
    { '/', { 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10 } },
    { '_', { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F } },
    { '+', { 0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00 } },
    { ':', { 0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00 } },
    { '<', { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02 } },
    { '>', { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08 } },
    { '?', { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04 } },
    { '!', { 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04 } },
    { '@', { 0x0E, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0E } },
    { '#', { 0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A } },
    { '$', { 0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04 } },
    { '%', { 0x19, 0x1A, 0x04, 0x08, 0x16, 0x06, 0x00 } },
    { '^', { 0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00 } },
    { '&', { 0x0C, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0D } },
    { '*', { 0x00, 0x15, 0x0E, 0x1F, 0x0E, 0x15, 0x00 } },
    { '(', { 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02 } },
    { ')', { 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08 } },
    { 'a', { 0x00, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F } },
    { 'b', { 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x1E } },
    { 'c', { 0x00, 0x00, 0x0F, 0x10, 0x10, 0x10, 0x0F } },
    { 'd', { 0x01, 0x01, 0x0D, 0x13, 0x11, 0x11, 0x0F } },
    { 'e', { 0x00, 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0F } },
    { 'f', { 0x06, 0x09, 0x08, 0x1C, 0x08, 0x08, 0x08 } },
    { 'g', { 0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x0E } },
    { 'h', { 0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x11 } },
    { 'i', { 0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x0E } },
    { 'j', { 0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0C } },
    { 'k', { 0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12 } },
    { 'l', { 0x0C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E } },
    { 'm', { 0x00, 0x00, 0x1A, 0x15, 0x15, 0x15, 0x15 } },
    { 'n', { 0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11 } },
    { 'o', { 0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E } },
    { 'p', { 0x00, 0x00, 0x1E, 0x11, 0x1E, 0x10, 0x10 } },
    { 'q', { 0x00, 0x00, 0x0F, 0x11, 0x0F, 0x01, 0x01 } },
    { 'r', { 0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10 } },
    { 's', { 0x00, 0x00, 0x0F, 0x10, 0x0E, 0x01, 0x1E } },
    { 't', { 0x08, 0x08, 0x1C, 0x08, 0x08, 0x09, 0x06 } },
    { 'u', { 0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0D } },
    { 'v', { 0x00, 0x00, 0x11, 0x11, 0x11, 0x0A, 0x04 } },
    { 'w', { 0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A } },
    { 'x', { 0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11 } },
    { 'y', { 0x00, 0x00, 0x11, 0x11, 0x0F, 0x01, 0x0E } },
    { 'z', { 0x00, 0x00, 0x1F, 0x02, 0x04, 0x08, 0x1F } },
    { 0x5B, { 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E } },
    { 0x5D, { 0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E } },
    { 0x5C, { 0x10, 0x08, 0x08, 0x04, 0x02, 0x02, 0x01 } },
    { 0x27, { 0x04, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00 } },
    { 0x22, { 0x0A, 0x0A, 0x04, 0x00, 0x00, 0x00, 0x00 } },
    { 0x7B, { 0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02 } },
    { 0x7D, { 0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08 } },
    { 0x7C, { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 } },
    { 0x60, { 0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00 } },
    { 0x7E, { 0x00, 0x00, 0x09, 0x16, 0x00, 0x00, 0x00 } }
};

#define OSK_FONT_COUNT (sizeof(osk_font) / sizeof(osk_font[0]))

enum {
    OSK_PAGE_ABC = 0,
    OSK_PAGE_FUNC = 1
};

/*
 * Page 1: normal US keyboard.
 * Shift supplies the symbols normally reached from this keyboard.
 */
static const char *osk_abc_rows[4] = {
    "1234567890-=",
    "QWERTYUIOP[]\\",
    "ASDFGHJKL;'",
    "ZXCVBNM,./`"
};

/*
 * Page 2: keys that cannot be reached from the compact ABC keyboard.
 *
 * Row 2/3 intentionally form a visual navigation cluster:
 *
 *        PRTSC        UP        PAUSE
 *        LEFT        DOWN       RIGHT
 */
static const char *osk_func_labels[4][12] = {
    {
        "F1", "F2", "F3", "F4",
        "F5", "F6", "F7", "F8",
        "F9", "F10", "F11", "F12"
    },
    {
        "HOME", "END", "PGUP", "PGDN", "INS", "CAPS"
    },
    {
        "PRTSC", "UP", "PAUSE"
    },
    {
        "LEFT", "DOWN", "RIGHT"
    }
};

static const unsigned int osk_func_keysyms[4][12] = {
    {
        XK_F1, XK_F2, XK_F3, XK_F4,
        XK_F5, XK_F6, XK_F7, XK_F8,
        XK_F9, XK_F10, XK_F11, XK_F12
    },
    {
        XK_HOME, XK_END, XK_PAGE_UP, XK_PAGE_DOWN,
        XK_INSERT, XK_CAPS_LOCK
    },
    {
        XK_PRINT, XK_UP, XK_PAUSE
    },
    {
        XK_LEFT, XK_DOWN, XK_RIGHT
    }
};

/*
 * Four content rows plus one permanent utility row.
 *
 * Bottom row on BOTH pages:
 *
 * ABC FUNC SHIFT CTRL ALT SPACE TAB BKSP DEL ENTER ESC
 */
static const int osk_page_row_lengths[2][OSK_ROWS] = {
    { 12, 13, 11, 11, 11 },  /* ABC  */
    { 12,  6,  3,  3, 11 }   /* FUNC */
};

static const char *osk_special_labels[11] = {
    "ABC", "FUNC",
    "SHIFT", "CTRL", "ALT",
    "SPACE", "TAB", "BKSP", "DEL", "ENTER", "ESC"
};

static int osk_row_length(int row)
{
    int page;

    if (row < 0 || row >= OSK_ROWS)
        return 0;

    page =
        (osk_page == OSK_PAGE_FUNC)
        ? OSK_PAGE_FUNC
        : OSK_PAGE_ABC;

    return osk_page_row_lengths[page][row];
}

static unsigned short osk_color(
    unsigned int r5,
    unsigned int g5,
    unsigned int b5)
{
    return (unsigned short)(
        0x8000 |
        ((b5 & 0x1F) << 10) |
        ((g5 & 0x1F) << 5) |
        (r5 & 0x1F)
    );
}

static void osk_fill_rect(
    int x,
    int y,
    int w,
    int h,
    unsigned short color)
{
    int yy;
    int xx;

    if (x < 0) {
        w += x;
        x = 0;
    }

    if (y < 0) {
        h += y;
        y = 0;
    }

    if (x + w > OSK_WIDTH)
        w = OSK_WIDTH - x;

    if (y + h > OSK_HEIGHT)
        h = OSK_HEIGHT - y;

    if (w <= 0 || h <= 0)
        return;

    for (yy = y; yy < y + h; yy++) {
        unsigned short *row =
            &osk_pixels[yy * OSK_WIDTH + x];

        for (xx = 0; xx < w; xx++)
            row[xx] = color;
    }
}

static const unsigned char *osk_find_glyph(char c)
{
    unsigned int i;

    for (i = 0; i < OSK_FONT_COUNT; i++) {
        if (osk_font[i].c == c)
            return osk_font[i].row;
    }

    return NULL;
}

static void osk_draw_char(
    int x,
    int y,
    char c,
    int scale,
    unsigned short color)
{
    const unsigned char *glyph = osk_find_glyph(c);
    int gy;
    int gx;

    if (glyph == NULL)
        return;

    for (gy = 0; gy < 7; gy++) {
        for (gx = 0; gx < 5; gx++) {
            if (glyph[gy] & (1 << (4 - gx))) {
                osk_fill_rect(
                    x + gx * scale,
                    y + gy * scale,
                    scale,
                    scale,
                    color
                );
            }
        }
    }
}

static void osk_draw_text_centered(
    int x,
    int y,
    int w,
    int h,
    const char *s,
    int scale,
    unsigned short color)
{
    int len = (int)strlen(s);
    int char_w = 6 * scale;
    int text_w = len * char_w - scale;
    int text_h = 7 * scale;
    int tx = x + (w - text_w) / 2;
    int ty = y + (h - text_h) / 2;
    int i;

    for (i = 0; i < len; i++) {
        osk_draw_char(tx + i * char_w, ty, s[i], scale, color);
    }
}

/*
 * Forward declaration: osk_display_char() uses this before its full
 * definition later in the file.
 */
static unsigned int osk_shifted_ascii(unsigned int c);

static char osk_display_char(int row, int col)
{
    unsigned int c;

    if (row < 0 || row >= 4)
        return ' ';

    c = (unsigned int)(unsigned char)
        osk_abc_rows[row][col];

    /*
     * Alphabet rows are stored uppercase for convenient layout.
     * With Shift off, send lowercase.
     */
    if (c >= 'A' && c <= 'Z')
        c = c - 'A' + 'a';

    if (osk_shift)
        c = osk_shifted_ascii(c);

    return (char)c;
}

static void osk_render_bitmap(void)
{
    const unsigned short panel = osk_color(3, 3, 4);
    const unsigned short key = osk_color(10, 10, 11);
    const unsigned short selected = osk_color(31, 24, 3);
    const unsigned short active = osk_color(6, 20, 8);
    const unsigned short border = osk_color(22, 22, 22);
    const unsigned short text_col = osk_color(31, 31, 31);
    const unsigned short selected_text = osk_color(1, 1, 1);

    int row;
    int col;
    int top = 4;
    int row_h = 33;

    osk_fill_rect(0, 0, OSK_WIDTH, OSK_HEIGHT, panel);

    for (row = 0; row < OSK_ROWS; row++) {
        int count = osk_row_length(row);
        int margin = 6;
        int gap = 3;
        int available =
            OSK_WIDTH - 2 * margin - (count - 1) * gap;
        int cell_w = available / count;
        int used_w =
            cell_w * count + gap * (count - 1);
        int row_x = (OSK_WIDTH - used_w) / 2;
        int y = top + row * row_h;
        int cell_h = row_h - 4;

        for (col = 0; col < count; col++) {
            int x = row_x + col * (cell_w + gap);

            int is_selected =
                (row == osk_row && col == osk_col);

            unsigned short bg =
                is_selected ? selected : key;

            unsigned short fg =
                is_selected ? selected_text : text_col;

            char label[8];
            int text_scale;

            if (row == 4) {
                strncpy(
                    label,
                    osk_special_labels[col],
                    sizeof(label) - 1
                );

                label[sizeof(label) - 1] = '\0';

                /*
                 * Persistent state indication:
                 *
                 * ABC / FUNC = active page
                 * Shift/Ctrl/Alt = latched modifier
                 *
                 * Cursor selection still takes visual priority.
                 */
                if (!is_selected &&
                    ((col == 0 &&
                      osk_page == OSK_PAGE_ABC) ||
                     (col == 1 &&
                      osk_page == OSK_PAGE_FUNC) ||
                     (col == 2 && osk_shift) ||
                     (col == 3 && osk_ctrl) ||
                     (col == 4 && osk_alt))) {
                    bg = active;
                }

            } else if (osk_page == OSK_PAGE_ABC) {
                label[0] =
                    osk_display_char(row, col);
                label[1] = '\0';

            } else {
                strncpy(
                    label,
                    osk_func_labels[row][col],
                    sizeof(label) - 1
                );

                label[sizeof(label) - 1] = '\0';
            }

            /*
             * Long labels use the compact font size.
             * F10/F11/F12 and the short utility keys still fit at 2x.
             */
            text_scale =
                (strlen(label) >= 5) ? 1 : 2;

            osk_fill_rect(
                x - 1, y - 1,
                cell_w + 2, cell_h + 2,
                border
            );

            osk_fill_rect(
                x, y,
                cell_w, cell_h,
                bg
            );

            osk_draw_text_centered(
                x, y,
                cell_w, cell_h,
                label,
                text_scale,
                fg
            );
        }
    }
}

static void display_confirm_render_bitmap(void)
{
    const unsigned short panel = osk_color(3, 3, 4);
    const unsigned short key = osk_color(8, 8, 10);
    const unsigned short selected = osk_color(8, 18, 31);
    const unsigned short text = osk_color(31, 31, 31);
    const unsigned short warn = osk_color(31, 24, 3);
    char line[80];

    osk_fill_rect(0, 0, OSK_WIDTH, OSK_HEIGHT, panel);

    osk_draw_text_centered(
        0, 10, OSK_WIDTH, 24,
        "KEEP THIS DISPLAY MODE?", 2, warn);

    snprintf(
        line, sizeof(line),
        "%s -> %s",
        display_confirm_previous_mode ?
            display_confirm_previous_mode->name : "?",
        active_video_mode ? active_video_mode->name : "?");

    osk_draw_text_centered(
        0, 37, OSK_WIDTH, 18,
        line, 1, text);

    osk_fill_rect(
        120, 62, OSK_WIDTH - 240, 27,
        display_confirm_selection == DISPLAY_CONFIRM_GO_BACK ?
            selected : key);

    osk_draw_text_centered(
        120, 62, OSK_WIDTH - 240, 27,
        "GO BACK", 2, text);

    osk_fill_rect(
        120, 95, OSK_WIDTH - 240, 27,
        display_confirm_selection == DISPLAY_CONFIRM_KEEP ?
            selected : key);

    osk_draw_text_centered(
        120, 95, OSK_WIDTH - 240, 27,
        "KEEP MODE", 2, text);

    snprintf(
        line, sizeof(line),
        "AUTO-REVERT IN %u SEC",
        display_confirm_seconds);

    osk_draw_text_centered(
        0, 133, OSK_WIDTH, 18,
        line, 1, text);

    osk_draw_text_centered(
        0, 153, OSK_WIDTH, 16,
        "X SELECT   O GO BACK", 1, text);
}


static void display_restore_ack_render_bitmap(void)
{
    const unsigned short panel = osk_color(3, 3, 4);
    const unsigned short key = osk_color(8, 18, 31);
    const unsigned short text = osk_color(31, 31, 31);
    const unsigned short good = osk_color(8, 31, 10);
    char line[80];

    osk_fill_rect(0, 0, OSK_WIDTH, OSK_HEIGHT, panel);

    osk_draw_text_centered(
        0, 14, OSK_WIDTH, 24,
        "ORIGINAL DISPLAY MODE RESTORED",
        2, good);

    snprintf(
        line,
        sizeof(line),
        "%s HAS BEEN RESTORED SUCCESSFULLY.",
        display_restore_ack_mode ?
            display_restore_ack_mode->name : "DISPLAY");

    osk_draw_text_centered(
        0, 54, OSK_WIDTH, 18,
        line, 1, text);

    osk_fill_rect(
        180, 91, OSK_WIDTH - 360, 29,
        key);

    osk_draw_text_centered(
        180, 91, OSK_WIDTH - 360, 29,
        "OK", 2, text);

    osk_draw_text_centered(
        0, 143, OSK_WIDTH, 18,
        "X OK", 1, text);
}



static int display_menu_tab_for_mode(
    const ps2vnc_video_mode_t *mode)
{
    if (mode == NULL)
        return DISPLAY_TAB_TV_DTV;

    switch (mode->menu_group) {
        case PS2VNC_DISPLAY_GROUP_TV_DTV:
            return DISPLAY_TAB_TV_DTV;

        case PS2VNC_DISPLAY_GROUP_VGA_60:
            return DISPLAY_TAB_VGA_60;

        case PS2VNC_DISPLAY_GROUP_VGA_OTHER:
            return DISPLAY_TAB_VGA_OTHER;

        default:
            return DISPLAY_TAB_TV_DTV;
    }
}


static const char *display_menu_tab_label(int tab)
{
    switch (tab) {
        case DISPLAY_TAB_TV_DTV:
            return "TV / DTV";

        case DISPLAY_TAB_VGA_60:
            return "VGA 60";

        case DISPLAY_TAB_VGA_OTHER:
            return "VGA OTHER";

        case DISPLAY_TAB_ADVANCED:
            return "ADVANCED";

        default:
            return "?";
    }
}


static int display_menu_mode_belongs_to_tab(
    int selection,
    int tab)
{
    const ps2vnc_video_mode_t *mode =
        display_menu_mode_at_selection(selection);

    if (mode == NULL ||
        tab < DISPLAY_TAB_TV_DTV ||
        tab > DISPLAY_TAB_VGA_OTHER)
        return 0;

    return display_menu_tab_for_mode(mode) == tab;
}


static int display_menu_mode_visible(int selection)
{
    const ps2vnc_video_mode_t *mode =
        display_menu_mode_at_selection(selection);

    if (mode == NULL)
        return 0;

    /*
     * The running mode is never hidden, even if an advanced/manual config
     * edit happens to mark it locked.
     */
    if (mode == active_video_mode)
        return 1;

    if (!display_hide_locked_modes)
        return 1;

    return display_mode_locked[selection] ? 0 : 1;
}


static int display_menu_first_visible_selection(int tab)
{
    unsigned int i;

    if (tab < DISPLAY_TAB_TV_DTV ||
        tab > DISPLAY_TAB_VGA_OTHER)
        return -1;

    for (i = 0; i < VIDEO_MODE_COUNT; i++) {
        if (display_menu_mode_belongs_to_tab((int)i, tab) &&
            display_menu_mode_visible((int)i))
            return (int)i;
    }

    return -1;
}


static int display_menu_tab_visible(int tab)
{
    if (tab == DISPLAY_TAB_ADVANCED)
        return 1;

    if (tab < DISPLAY_TAB_TV_DTV ||
        tab > DISPLAY_TAB_VGA_OTHER)
        return 0;

    return display_menu_first_visible_selection(tab) >= 0;
}


static int display_menu_next_visible_tab(
    int tab,
    int direction)
{
    int step;

    if (direction == 0)
        return display_menu_tab_visible(tab)
            ? tab
            : DISPLAY_TAB_ADVANCED;

    for (step = 1;
         step <= DISPLAY_TAB_COUNT;
         step++) {

        int candidate =
            (tab +
             direction * step +
             DISPLAY_TAB_COUNT * 4) %
            DISPLAY_TAB_COUNT;

        if (display_menu_tab_visible(candidate))
            return candidate;
    }

    return DISPLAY_TAB_ADVANCED;
}


static void display_menu_normalize_selection(void)
{
    int saved;

    if (!display_menu_tab_visible(display_menu_tab))
        display_menu_tab =
            display_menu_next_visible_tab(
                display_menu_tab,
                1);

    if (display_menu_tab == DISPLAY_TAB_ADVANCED) {
        display_menu_selection = -1;
        return;
    }

    if (display_menu_mode_belongs_to_tab(
            display_menu_selection,
            display_menu_tab) &&
        display_menu_mode_visible(display_menu_selection)) {

        display_menu_tab_selection[display_menu_tab] =
            display_menu_selection;

        return;
    }

    saved = display_menu_tab_selection[display_menu_tab];

    if (display_menu_mode_belongs_to_tab(
            saved,
            display_menu_tab) &&
        display_menu_mode_visible(saved)) {

        display_menu_selection = saved;
        return;
    }

    display_menu_selection =
        display_menu_first_visible_selection(
            display_menu_tab);

    if (display_menu_selection >= 0)
        display_menu_tab_selection[display_menu_tab] =
            display_menu_selection;
}


static void display_menu_open_for_active(int parent_system)
{
    int active_selection =
        display_menu_selection_for_mode(active_video_mode);

    display_menu_tab =
        display_menu_tab_for_mode(active_video_mode);

    display_menu_selection = active_selection;

    if (display_menu_tab >= DISPLAY_TAB_TV_DTV &&
        display_menu_tab <= DISPLAY_TAB_VGA_OTHER) {

        display_menu_tab_selection[display_menu_tab] =
            active_selection;
    }

    display_menu_error = 0;
    display_policy_error = 0;
    display_menu_parent_system = parent_system;
    display_menu_visible = 1;

    /*
     * Re-read authoritative policy every time the selector is opened.
     * Until the main thread completes that GET, switching into inactive
     * modes remains disabled.
     */
    if (display_policy_request ==
            DISPLAY_POLICY_REQUEST_NONE) {

        display_policy_loaded = 0;
        display_policy_request =
            DISPLAY_POLICY_REQUEST_REFRESH;
    }

    osk_generation++;
}


static int display_menu_move_vertical_selection(
    int selection,
    int direction)
{
    int step;

    if (display_menu_tab == DISPLAY_TAB_ADVANCED ||
        direction == 0)
        return selection;

    if (!display_menu_mode_belongs_to_tab(
            selection,
            display_menu_tab) ||
        !display_menu_mode_visible(selection)) {

        return display_menu_first_visible_selection(
            display_menu_tab);
    }

    for (step = 1;
         step <= (int)VIDEO_MODE_COUNT;
         step++) {

        int candidate =
            (selection +
             direction * step +
             (int)VIDEO_MODE_COUNT * 4) %
            (int)VIDEO_MODE_COUNT;

        if (display_menu_mode_belongs_to_tab(
                candidate,
                display_menu_tab) &&
            display_menu_mode_visible(candidate)) {

            display_menu_tab_selection[display_menu_tab] =
                candidate;

            return candidate;
        }
    }

    return selection;
}


static int display_menu_move_horizontal_selection(
    int selection,
    int direction)
{
    int old_tab = display_menu_tab;

    (void)selection;

    if (direction == 0)
        return display_menu_selection;

    if (old_tab >= DISPLAY_TAB_TV_DTV &&
        old_tab <= DISPLAY_TAB_VGA_OTHER &&
        display_menu_mode_belongs_to_tab(
            display_menu_selection,
            old_tab)) {

        display_menu_tab_selection[old_tab] =
            display_menu_selection;
    }

    display_menu_tab =
        display_menu_next_visible_tab(
            display_menu_tab,
            direction);

    display_menu_normalize_selection();

    return display_menu_selection;
}


static void display_menu_render_bitmap(void)
{
    const unsigned short panel =
        osk_color(3, 3, 4);

    const unsigned short key =
        osk_color(10, 10, 11);

    const unsigned short selected =
        osk_color(31, 24, 3);

    const unsigned short active =
        osk_color(6, 20, 8);

    const unsigned short locked_bg =
        osk_color(5, 5, 6);

    const unsigned short border =
        osk_color(22, 22, 22);

    const unsigned short text_col =
        osk_color(31, 31, 31);

    const unsigned short locked_text =
        osk_color(18, 18, 18);

    const unsigned short selected_text =
        osk_color(1, 1, 1);

    const int margin = 4;
    const int tab_gap = 2;
    const int title_y = 2;
    const int tab_y = 16;
    const int tab_h = 12;

    const int list_x = 55;
    const int list_w = OSK_WIDTH - 110;
    const int top = 37;
    const int row_h = 10;
    const int cell_h = 9;

    const int footer_y = 159;

    int visible_tab_count = 0;
    int tab_w;
    int tab;
    int tab_draw_index = 0;

    for (tab = 0;
         tab < DISPLAY_TAB_COUNT;
         tab++) {

        if (display_menu_tab_visible(tab))
            visible_tab_count++;
    }

    /*
     * ADVANCED is permanently visible, so this is defensive only.
     */
    if (visible_tab_count <= 0)
        visible_tab_count = 1;

    tab_w =
        (OSK_WIDTH -
         margin * 2 -
         tab_gap * (visible_tab_count - 1)) /
        visible_tab_count;

    osk_fill_rect(
        0, 0,
        OSK_WIDTH, OSK_HEIGHT,
        panel
    );

    osk_draw_text_centered(
        0, title_y,
        OSK_WIDTH, 12,
        "DISPLAY SETTINGS",
        1,
        text_col
    );

    /*
     * Empty mode groups disappear from presentation. ADVANCED remains
     * permanently visible so Hide Locked Modes can always be reversed.
     */
    for (tab = 0;
         tab < DISPLAY_TAB_COUNT;
         tab++) {

        int x;

        if (!display_menu_tab_visible(tab))
            continue;

        x =
            margin +
            tab_draw_index * (tab_w + tab_gap);

        tab_draw_index++;

        unsigned short bg =
            (tab == display_menu_tab)
            ? selected
            : key;

        unsigned short fg =
            (tab == display_menu_tab)
            ? selected_text
            : text_col;

        osk_fill_rect(
            x, tab_y - 1,
            tab_w, tab_h + 2,
            border
        );

        osk_fill_rect(
            x + 1, tab_y,
            tab_w - 2, tab_h,
            bg
        );

        osk_draw_text_centered(
            x + 1, tab_y,
            tab_w - 2, tab_h,
            display_menu_tab_label(tab),
            1,
            fg
        );
    }

    if (display_menu_tab == DISPLAY_TAB_ADVANCED) {
        char setting[96];

        snprintf(
            setting,
            sizeof(setting),
            "HIDE LOCKED MODES: %s",
            display_hide_locked_modes
                ? "ON"
                : "OFF"
        );

        osk_fill_rect(
            list_x,
            55,
            list_w,
            20,
            border
        );

        osk_fill_rect(
            list_x + 1,
            56,
            list_w - 2,
            18,
            selected
        );

        osk_draw_text_centered(
            list_x + 1,
            56,
            list_w - 2,
            18,
            setting,
            1,
            selected_text
        );

    } else {
        unsigned int i;
        int row = 0;

        for (i = 0;
             i < VIDEO_MODE_COUNT;
             i++) {

            const ps2vnc_video_mode_t *mode =
                video_modes[i];

            unsigned short bg;
            unsigned short fg;
            char row_text[96];
            int y;

            if (!display_menu_mode_belongs_to_tab(
                    (int)i,
                    display_menu_tab))
                continue;

            if (!display_menu_mode_visible((int)i))
                continue;

            y = top + row * row_h;

            if (mode == active_video_mode) {
                bg = active;
                fg = text_col;

            } else if (display_mode_locked[i]) {
                bg = locked_bg;
                fg = locked_text;

            } else {
                bg = key;
                fg = text_col;
            }

            if ((int)i ==
                    display_menu_selection) {

                bg = selected;
                fg = selected_text;
            }

            if (mode == active_video_mode &&
                display_mode_locked[i]) {

                snprintf(
                    row_text,
                    sizeof(row_text),
                    "%s   CURRENT   LOCK",
                    mode->name
                );

            } else if (mode == active_video_mode) {

                snprintf(
                    row_text,
                    sizeof(row_text),
                    "%s   CURRENT",
                    mode->name
                );

            } else if (display_mode_locked[i]) {

                snprintf(
                    row_text,
                    sizeof(row_text),
                    "%s   LOCK",
                    mode->name
                );

            } else {

                snprintf(
                    row_text,
                    sizeof(row_text),
                    "%s",
                    mode->name
                );
            }

            osk_fill_rect(
                list_x,
                y - 1,
                list_w,
                cell_h + 2,
                border
            );

            osk_fill_rect(
                list_x + 1,
                y,
                list_w - 2,
                cell_h,
                bg
            );

            osk_draw_text_centered(
                list_x + 1,
                y,
                list_w - 2,
                cell_h,
                row_text,
                1,
                fg
            );

            row++;
        }

        if (row == 0) {
            osk_draw_text_centered(
                0, 70,
                OSK_WIDTH, 14,
                "NO VISIBLE MODES",
                1,
                text_col
            );
        }
    }

    /*
     * Request status owns the footer while a main-thread management
     * operation is in progress.
     */
    if (display_policy_request !=
            DISPLAY_POLICY_REQUEST_NONE) {

        osk_draw_text_centered(
            0, footer_y,
            OSK_WIDTH, 12,
            display_policy_request ==
                DISPLAY_POLICY_REQUEST_REFRESH
                ? "LOADING DISPLAY SETTINGS..."
                : "SAVING DISPLAY SETTINGS...",
            1,
            text_col
        );

    } else if (display_policy_error) {

        osk_draw_text_centered(
            0, footer_y,
            OSK_WIDTH, 12,
            "DISPLAY SETTINGS UNAVAILABLE   O BACK",
            1,
            text_col
        );

    } else if (display_menu_error) {

        osk_draw_text_centered(
            0, footer_y,
            OSK_WIDTH, 12,
            "PRESET UNAVAILABLE   O BACK",
            1,
            text_col
        );

    } else if (display_menu_tab ==
                   DISPLAY_TAB_ADVANCED) {

        osk_draw_text_centered(
            0, footer_y,
            OSK_WIDTH, 12,
            "X TOGGLE   O BACK",
            1,
            text_col
        );

    } else {
        const ps2vnc_video_mode_t *selected_mode =
            display_menu_mode_at_selection(
                display_menu_selection);

        char footer[128];

        if (selected_mode != NULL &&
            selected_mode == active_video_mode) {

            snprintf(
                footer,
                sizeof(footer),
                "X CALIBRATE %s SAFE AREA   O BACK",
                selected_mode->name
            );

        } else if (selected_mode != NULL &&
                   display_mode_locked[
                       display_menu_selection]) {

            snprintf(
                footer,
                sizeof(footer),
                "SQUARE UNLOCK   X LOCKED   O BACK"
            );

        } else if (selected_mode != NULL) {

            snprintf(
                footer,
                sizeof(footer),
                "SQUARE LOCK   X SWITCH TO %s   O BACK",
                selected_mode->name
            );

        } else {

            snprintf(
                footer,
                sizeof(footer),
                "O BACK"
            );
        }

        osk_draw_text_centered(
            0, footer_y,
            OSK_WIDTH, 12,
            footer,
            1,
            text_col
        );
    }
}



static void system_menu_render_bitmap(void)
{
    const unsigned short panel = osk_color(3, 3, 4);
    const unsigned short key = osk_color(10, 10, 11);
    const unsigned short selected = osk_color(31, 24, 3);
    const unsigned short border = osk_color(22, 22, 22);
    const unsigned short text_col = osk_color(31, 31, 31);
    const unsigned short selected_text = osk_color(1, 1, 1);

    static const char *labels[SYSTEM_MENU_ITEM_COUNT] = {
        "REFRESH VNC SESSION",
        "DISPLAY SETTINGS",
        "EXIT TO SYSTEM MENU"
    };

    char footer[64];
    int i;

    osk_fill_rect(0, 0, OSK_WIDTH, OSK_HEIGHT, panel);

    osk_draw_text_centered(
        0, 5, OSK_WIDTH, 20,
        "PS2VNC SYSTEM", 2, text_col
    );

    for (i = 0; i < SYSTEM_MENU_ITEM_COUNT; i++) {
        int y = 32 + i * 29;
        unsigned short bg =
            (i == system_menu_selection) ? selected : key;
        unsigned short fg =
            (i == system_menu_selection) ? selected_text : text_col;

        osk_fill_rect(
            89, y - 1,
            OSK_WIDTH - 178, 25,
            border
        );

        osk_fill_rect(
            90, y,
            OSK_WIDTH - 180, 23,
            bg
        );

        osk_draw_text_centered(
            90, y,
            OSK_WIDTH - 180, 23,
            labels[i], 1, fg
        );
    }

    footer[0] = '\0';

    if (system_menu_countdown_action ==
            RUNTIME_ACTION_REFRESH_RFB &&
        system_menu_countdown_seconds > 0) {

        snprintf(
            footer, sizeof(footer),
            "HOLD FOR REFRESH: %u",
            system_menu_countdown_seconds
        );

    } else if (system_menu_countdown_action ==
                   RUNTIME_ACTION_EXIT_TO_SYSTEM_MENU &&
               system_menu_countdown_seconds > 0) {

        snprintf(
            footer, sizeof(footer),
            "HOLD FOR EXIT: %u",
            system_menu_countdown_seconds
        );

    } else if (system_refresh_state ==
                   SYSTEM_REFRESH_REFRESHING) {

        strncpy(
            footer,
            "REFRESHING VNC SESSION...",
            sizeof(footer) - 1
        );
        footer[sizeof(footer) - 1] = '\0';

    } else if (system_refresh_state ==
                   SYSTEM_REFRESH_COOLDOWN) {

        strncpy(
            footer,
            "REFRESH COMPLETE - COOLDOWN",
            sizeof(footer) - 1
        );
        footer[sizeof(footer) - 1] = '\0';

    } else {
        strncpy(
            footer,
            "REFRESH READY   X SELECT   O BACK",
            sizeof(footer) - 1
        );
        footer[sizeof(footer) - 1] = '\0';
    }

    osk_draw_text_centered(
        0, 121, OSK_WIDTH, 16,
        footer, 1, text_col
    );
}


static void osk_move_horizontal(int direction)
{
    int count = osk_row_length(osk_row);

    osk_col += direction;

    if (osk_col < 0)
        osk_col = count - 1;
    else if (osk_col >= count)
        osk_col = 0;

    osk_generation++;
}

static void osk_move_vertical(int direction)
{
    int old_count = osk_row_length(osk_row);
    int old_col = osk_col;
    int new_row = osk_row + direction;
    int new_count;

    if (new_row < 0)
        new_row = OSK_ROWS - 1;
    else if (new_row >= OSK_ROWS)
        new_row = 0;

    new_count = osk_row_length(new_row);

    /*
     * Preserve approximate horizontal position when crossing rows
     * with different numbers of keys.
     */
    osk_col =
        (old_col * new_count) / old_count;

    if (osk_col >= new_count)
        osk_col = new_count - 1;

    osk_row = new_row;
    osk_generation++;
}

static unsigned int osk_shifted_ascii(unsigned int c)
{
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 'A';

    if (c >= 'A' && c <= 'Z')
        return c;

    switch (c) {
        case '1': return '!';
        case '2': return '@';
        case '3': return '#';
        case '4': return '$';
        case '5': return '%';
        case '6': return '^';
        case '7': return '&';
        case '8': return '*';
        case '9': return '(';
        case '0': return ')';
        case '-': return '_';
        case '=': return '+';
        case ';': return ':';
        case ',': return '<';
        case '.': return '>';
        case '/': return '?';
        case 0x5B: return 0x7B; /* [ -> { */
        case 0x5D: return 0x7D; /* ] -> } */
        case 0x5C: return 0x7C; /* backslash -> | */
        case 0x27: return 0x22; /* apostrophe -> quote */
        case 0x60: return 0x7E; /* backtick -> ~ */
        default:  return c;
    }
}

static int osk_send_key_with_modifiers(
    int sock,
    unsigned int keysym)
{
    if (send_modified_key_tap(
            sock,
            keysym,
            osk_ctrl,
            osk_alt) < 0)
        return -1;

    /*
     * Shift/Ctrl/Alt are all one-shot.  After the next actual key is sent,
     * every latched modifier returns to off.
     */
    osk_clear_one_shot_modifiers();
    return 0;
}

static int osk_send_extended_key_with_modifiers(
    int sock,
    unsigned int keysym)
{
    int use_shift = osk_shift;
    int result;

    if (use_shift) {
        if (send_key_event(
                sock,
                1,
                XK_SHIFT_L) < 0)
            return -1;
    }

    result =
        send_modified_key_tap(
            sock,
            keysym,
            osk_ctrl,
            osk_alt
        );

    if (use_shift) {
        /*
         * Always attempt the release once Shift-down was queued.
         */
        if (send_key_event(
                sock,
                0,
                XK_SHIFT_L) < 0)
            return -1;
    }

    if (result < 0)
        return -1;

    osk_clear_one_shot_modifiers();
    return 0;
}


static int osk_activate_selected(int sock)
{
    unsigned int keysym;

    if (osk_row < 4) {
        if (osk_page == OSK_PAGE_ABC) {
            /*
             * ABC keys send the exact displayed printable character.
             */
            keysym =
                (unsigned int)(unsigned char)
                osk_display_char(
                    osk_row,
                    osk_col
                );

            return
                osk_send_key_with_modifiers(
                    sock,
                    keysym
                );
        }

        /*
         * FUNC page contains real X11 non-printable keysyms.
         */
        keysym =
            osk_func_keysyms
                [osk_row]
                [osk_col];

        if (keysym == 0)
            return 0;

        return
            osk_send_extended_key_with_modifiers(
                sock,
                keysym
            );
    }

    /*
     * Permanent utility row:
     *
     * ABC FUNC SHIFT CTRL ALT SPACE TAB BKSP DEL ENTER ESC
     */
    switch (osk_col) {
        case 0: /* ABC */
            if (osk_page != OSK_PAGE_ABC) {
                osk_page = OSK_PAGE_ABC;
                osk_generation++;
            }
            return 0;

        case 1: /* FUNC */
            if (osk_page != OSK_PAGE_FUNC) {
                osk_page = OSK_PAGE_FUNC;
                osk_generation++;
            }
            return 0;

        case 2: /* SHIFT */
            osk_shift = !osk_shift;
            osk_generation++;
            return 0;

        case 3: /* CTRL */
            osk_ctrl = !osk_ctrl;
            osk_generation++;
            return 0;

        case 4: /* ALT */
            osk_alt = !osk_alt;
            osk_generation++;
            return 0;

        case 5: /* SPACE */
            keysym = ' ';
            break;

        case 6: /* TAB */
            keysym = XK_TAB;
            break;

        case 7: /* BKSP */
            keysym = XK_BACKSPACE;
            break;

        case 8: /* DEL */
            keysym = XK_DELETE;
            break;

        case 9: /* ENTER */
            keysym = XK_RETURN;
            break;

        case 10: /* ESC */
            keysym = XK_ESCAPE;
            break;

        default:
            return 0;
    }

    /*
     * Utility keys use the extended sender so Shift+Tab and similar
     * combinations work as true modifier combinations.
     */
    return
        osk_send_extended_key_with_modifiers(
            sock,
            keysym
        );
}

/*
 * The stable Milestone-7/8 receive loop blocks waiting for a changed
 * framebuffer.  OSK navigation itself changes only local PS2 state, so
 * nudge the remote cursor by one pixel and back to wake the outstanding
 * incremental framebuffer request.  The pointer ends exactly where it
 * started.
 */
static int wake_renderer_with_pointer_jiggle(
    int sock,
    int x,
    int y)
{
    int jiggle_x;

    if (x < desktop_width - 1)
        jiggle_x = x + 1;
    else
        jiggle_x = x - 1;

    if (send_pointer_event(sock, 0,
                           (unsigned int)jiggle_x,
                           (unsigned int)y) < 0)
        return -1;

    if (send_pointer_event(sock, 0,
                           (unsigned int)x,
                           (unsigned int)y) < 0)
        return -1;

    return 0;
}

/*
 * Test11K-C shared analog response curve.
 *
 * The first ~85% of physical stick travel is deliberately shallow and
 * reaches only about 30% of maximum speed.  The final ~15% supplies the
 * remaining acceleration.
 *
 * Return value is Q12:
 *
 *     0    = stopped
 *     1229 = ~30%
 *     4096 = 100%
 */
#define ANALOG_DEADZONE       10
#define ANALOG_HALF_RAW       64   /* ~50% physical stick tilt */
#define ANALOG_KNEE_RAW       108  /* ~85% physical stick tilt */
#define ANALOG_MAX_RAW        127

#define ANALOG_CURVE_ONE      4096
#define ANALOG_CURVE_KNEE     1229  /* ~30% of 4096 */

static unsigned int analog_speed_curve_q12(int raw_magnitude)
{
    int usable;
    int knee_usable;
    int max_usable;
    unsigned int curve;

    if (raw_magnitude <= ANALOG_DEADZONE)
        return 0;

    if (raw_magnitude > ANALOG_MAX_RAW)
        raw_magnitude = ANALOG_MAX_RAW;

    usable =
        raw_magnitude - ANALOG_DEADZONE;

    knee_usable =
        ANALOG_KNEE_RAW - ANALOG_DEADZONE;

    max_usable =
        ANALOG_MAX_RAW - ANALOG_DEADZONE;

    if (usable <= knee_usable) {
        unsigned int scale_q12;

        /*
         * Base 11K-C shallow quadratic:
         *
         *     85% physical tilt = ~30% maximum speed.
         */
        curve =
            (ANALOG_CURVE_KNEE *
             (unsigned int)usable *
             (unsigned int)usable) /
            ((unsigned int)knee_usable *
             (unsigned int)knee_usable);

        /*
         * Test11K-D low-end refinement.
         *
         * Through the first 50% of physical stick travel, run at
         * exactly HALF of the previous 11K-C response.
         *
         * From 50% through the 85% knee, smoothly remove that
         * reduction so we arrive at the original 30% knee speed
         * without a discontinuity.
         */
        if (raw_magnitude <= ANALOG_HALF_RAW) {
            scale_q12 = 2048;  /* 50% */
        } else {
            scale_q12 =
                2048 +
                (2048 *
                 (unsigned int)
                 (raw_magnitude - ANALOG_HALF_RAW)) /
                (unsigned int)
                (ANALOG_KNEE_RAW - ANALOG_HALF_RAW);
        }

        curve =
            (curve * scale_q12) /
            ANALOG_CURVE_ONE;

        if (curve == 0)
            curve = 1;

        return curve;
    }

    /*
     * Final ~15% remains exactly as before:
     * 30% at the knee -> 100% at full tilt.
     */
    return
        ANALOG_CURVE_KNEE +
        ((ANALOG_CURVE_ONE - ANALOG_CURVE_KNEE) *
         (unsigned int)(usable - knee_usable)) /
        (unsigned int)(max_usable - knee_usable);
}


/*
 * Cursor velocity in Q8.
 *
 * 256  = 1 pixel per controller poll
 * 1536 = 6 pixels per poll at full tilt
 */
static int stick_velocity_q8(unsigned char value)
{
    int d = (int)value - 128;
    int sign;
    int raw;
    unsigned int curve;
    int velocity;

    if (d < 0) {
        sign = -1;
        raw = -d;
    } else {
        sign = 1;
        raw = d;
    }

    curve = analog_speed_curve_q12(raw);

    if (curve == 0)
        return 0;

    velocity =
        (1536 * (int)curve) /
        ANALOG_CURVE_ONE;

    /*
     * Less than one pixel per poll is intentional.
     * Q8 accumulation below lets this become genuinely slow motion.
     */
    /*
     * Permit extremely fine motion near center.
     * Q8 value 2 is about 0.47 pixel/sec at a 60 Hz poll rate.
     */
    if (velocity < 2)
        velocity = 2;

    return sign * velocity;
}

/*
 * Test11K controller scrolling.
 *
 * RFB PointerEvent button bits follow the conventional X11 mouse mapping:
 *
 *   button 4 = wheel up       = 0x08
 *   button 5 = wheel down     = 0x10
 *   button 6 = wheel left     = 0x20
 *   button 7 = wheel right    = 0x40
 *
 * A wheel "click" is represented as a button press followed immediately
 * by release at the same cursor position.
 */
#define RFB_SCROLL_UP     0x08
#define RFB_SCROLL_DOWN   0x10
#define RFB_SCROLL_LEFT   0x20
#define RFB_SCROLL_RIGHT  0x40

#define DPAD_SCROLL_REPEAT_POLLS  5


static unsigned char scroll_button_for_stick(
    unsigned char h,
    unsigned char v,
    int *magnitude)
{
    int x = (int)h - 128;
    int y = (int)v - 128;
    int ax = (x < 0) ? -x : x;
    int ay = (y < 0) ? -y : y;

    *magnitude = 0;

    if (ax <= ANALOG_DEADZONE &&
        ay <= ANALOG_DEADZONE)
        return 0;

    /*
     * Dominant axis only.
     */
    if (ay >= ax) {
        *magnitude = ay;

        if (y < 0)
            return RFB_SCROLL_UP;

        return RFB_SCROLL_DOWN;
    }

    *magnitude = ax;

    if (x < 0)
        return RFB_SCROLL_LEFT;

    return RFB_SCROLL_RIGHT;
}


/*
 * Number of 16.7 ms controller polls to wait between wheel clicks.
 *
 * Stronger stick deflection therefore scrolls faster.
 */
static unsigned int scroll_repeat_delay(int magnitude)
{
    unsigned int curve;
    unsigned int clicks_per_second_x10;
    unsigned int interval_polls;

    curve = analog_speed_curve_q12(magnitude);

    if (curve == 0)
        return 59;

    /*
     * Full tilt = ~20 wheel clicks/sec.
     *
     * At the 85% knee, the common curve is 30%, giving ~6 clicks/sec.
     *
     * Near center we floor at ~1 click/sec rather than making scrolling
     * disappear completely.
     */
    clicks_per_second_x10 =
        (200 * curve) /
        ANALOG_CURVE_ONE;

    /*
     * Near center allow scrolling as slowly as one wheel notch
     * about every two seconds.
     */
    if (clicks_per_second_x10 < 5)
        clicks_per_second_x10 = 5;

    /*
     * Controller polls at about 60 Hz.
     * x10 arithmetic keeps useful precision at low scroll speeds.
     */
    interval_polls =
        (600 + clicks_per_second_x10 / 2) /
        clicks_per_second_x10;

    if (interval_polls < 3)
        interval_polls = 3;

    return interval_polls - 1;
}


static int send_scroll_pulse(
    int sock,
    unsigned char base_buttons,
    unsigned char scroll_button,
    int x,
    int y)
{
    /*
     * Preserve any ordinary mouse button already held while adding the
     * momentary wheel bit.
     */
    if (send_pointer_event(
            sock,
            base_buttons | scroll_button,
            (unsigned int)x,
            (unsigned int)y) < 0)
        return -1;

    if (send_pointer_event(
            sock,
            base_buttons,
            (unsigned int)x,
            (unsigned int)y) < 0)
        return -1;

    return 0;
}


/*
 * Test11K-G2 progressive D-pad acceleration.
 *
 * Controller loop is approximately 60 Hz.
 *
 * For the first second the D-pad retains its existing precision behavior.
 * After one second, elapsed hold time is mapped onto the SAME response
 * curve used by the analog stick.
 *
 * This gives:
 *
 *   tap       = 2 pixels
 *   short hold = gentle precision repeat
 *   >1 sec    = gradual analog-like acceleration
 *   ~2.5 sec  = full analog-stick top speed
 */
#define DPAD_ACCEL_START_TICKS   60
#define DPAD_ACCEL_RAMP_TICKS    90

#define DPAD_BASE_VELOCITY_Q8    128
#define DPAD_MAX_VELOCITY_Q8     1536

static int dpad_hold_velocity_q8(unsigned int hold_ticks)
{
    unsigned int t;
    int raw;
    unsigned int curve;

    if (hold_ticks <= DPAD_ACCEL_START_TICKS)
        return DPAD_BASE_VELOCITY_Q8;

    t = hold_ticks - DPAD_ACCEL_START_TICKS;

    if (t >= DPAD_ACCEL_RAMP_TICKS)
        return DPAD_MAX_VELOCITY_Q8;

    /*
     * Convert elapsed acceleration time into an equivalent
     * analog-stick magnitude.
     *
     * t = 0                  -> deadzone edge
     * t = DPAD_ACCEL_RAMP    -> full stick
     */
    raw =
        ANALOG_DEADZONE +
        ((ANALOG_MAX_RAW - ANALOG_DEADZONE) *
         (int)t) /
        DPAD_ACCEL_RAMP_TICKS;

    curve = analog_speed_curve_q12(raw);

    /*
     * Start exactly at the existing held-D-pad average speed:
     *
     *     2 px / 4 polls = 0.5 px/poll = Q8 128
     *
     * Then accelerate smoothly to the analog maximum of
     * 6 px/poll = Q8 1536.
     */
    return
        DPAD_BASE_VELOCITY_Q8 +
        ((DPAD_MAX_VELOCITY_Q8 -
          DPAD_BASE_VELOCITY_Q8) *
         (int)curve) /
        ANALOG_CURVE_ONE;
}


static void controller_thread(void *arg)
{
    int sock = *(int *)arg;
    int cursor_x = desktop_width / 2;
    int cursor_y = desktop_height / 2;
    unsigned char last_button_mask = 0;
    unsigned int last_pressed = 0;

    /*
     * Precision D-pad mouse movement:
     * immediate 2px step, then delayed repeat if held.
     */
    unsigned int dpad_hold_direction = 0;
    unsigned int dpad_hold_ticks = 0;

    /*
     * Fractional Q8 movement for accelerated D-pad motion.
     */
    int dpad_x_q8 = 0;
    int dpad_y_q8 = 0;

    /*
     * Fractional Q8 cursor motion.
     *
     * This is what permits genuinely slow movement near stick center:
     * several controller polls may be required to accumulate one pixel.
     */
    int analog_x_q8 = 0;
    int analog_y_q8 = 0;

    /*
     * Scroll repeat state survives from one controller poll to the next.
     */
    unsigned char last_scroll_mask = 0;
    unsigned int scroll_repeat_countdown = 0;
    unsigned int hotkey_observed_mask = 0;
    unsigned int hotkey_stable_polls = 0;
    int hotkey_active_action = RUNTIME_ACTION_NONE;
    int hotkey_active_trigger = 0;
    unsigned int hotkey_hold_polls = 0;
    int hotkey_hold_last_seconds = -1;
    unsigned int hotkey_release_armed_mask = 0;
    int hotkey_release_armed_action = RUNTIME_ACTION_NONE;
    int hotkey_latched = 0;
    int hotkey_desktop_session = 0;

    /*
     * Buttons consumed by a local PS2VNC UI stay quarantined from
     * remote-desktop input until those physical buttons are released.
     *
     * This is stronger than same-poll consumption: a Circle press that
     * closes a menu may still be physically down on the next controller
     * poll, when the desktop branch has become active again.
     */
    unsigned int local_ui_quarantine = 0;

    const struct timespec poll_delay = {
        0,
        16667000
    };

    for (;;) {
        struct padButtonStatus buttons;
        unsigned int pressed;
        unsigned int changed;
        unsigned char button_mask;
        int old_x = cursor_x;
        int old_y = cursor_y;
        int dx = 0;
        int dy = 0;

        unsigned char scroll_mask = 0;
        unsigned int scroll_delay = 0;
        int scroll_magnitude = 0;

        /*
         * G2-C pad ownership handoff.
         *
         * The acknowledgement is published only after this thread has
         * reached a point immediately before its next libpad access.
         * While acknowledged, this thread performs no padGetState/padRead
         * calls and therefore the main thread may temporarily own libpad.
         */
        if (controller_pad_pause_requested) {
            controller_pad_pause_ack = 1;

            while (controller_pad_pause_requested &&
                   !exit_requested) {
                nanosleep(&poll_delay, NULL);
            }

            controller_pad_pause_ack = 0;

            if (exit_requested) {
                ExitThread();
                return;
            }

            /*
             * The main thread will return ownership only after a clean
             * physical release. Forget every pre-handoff edge/hold state so
             * no calibration button can become desktop input afterward.
             */
            last_pressed = 0;
            last_button_mask = 0;

            dpad_hold_direction = 0;
            dpad_hold_ticks = 0;
            dpad_x_q8 = 0;
            dpad_y_q8 = 0;

            analog_x_q8 = 0;
            analog_y_q8 = 0;

            last_scroll_mask = 0;
            scroll_repeat_countdown = 0;

            hotkey_observed_mask = 0;
            hotkey_stable_polls = 0;
            hotkey_active_action = RUNTIME_ACTION_NONE;
            hotkey_active_trigger = 0;
            hotkey_hold_polls = 0;
            hotkey_hold_last_seconds = -1;
            hotkey_release_armed_mask = 0;
            hotkey_release_armed_action = RUNTIME_ACTION_NONE;
            hotkey_latched = 0;
            hotkey_desktop_session = 0;

            local_ui_quarantine = 0;

            continue;
        }

        controller_loop_counter++;

        if (ps2vnc_system_refresh_update_state() &&
            system_menu_visible) {

            (void)wake_renderer_with_pointer_jiggle(
                sock, cursor_x, cursor_y);
        }

        if (padGetState(0, 0) == PAD_STATE_STABLE &&
            padRead(0, 0, &buttons) != 0) {

            pressed = 0xFFFFu ^ (unsigned int)buttons.btns;
            changed = pressed ^ last_pressed;

            /*
             * Release is the only thing that clears local ownership.
             * Held buttons therefore cannot leak into the remote desktop
             * after a local menu closes.
             */
            local_ui_quarantine &= pressed;

            /*
             * TEST15E3-G generic runtime-hotkey arbitration.
             *
             * Only buttons participating in runtime bindings are sampled.
             * The exact held mask must remain unchanged for a short settle
             * interval before a multi-button binding owns the session.
             *
             * Once an action actually fires, the hotkey session remains
             * latched until all hotkey buttons are released.  Therefore a
             * shrinking chord cannot accidentally fire one of its subsets.
             *
             * Desktop-only bindings never delay OSK or local-menu controls.
             * Global Refresh/Exit bindings remain observable in every context.
             */
            {
                unsigned int hk =
                    pressed & RUNTIME_HOTKEY_BUTTON_MASK;

                int desktop_hotkey_context =
                    !system_menu_visible &&
                    !display_menu_visible &&
                    !osk_visible;

                if (hotkey_latched) {
                    if (hk == 0) {
                        hotkey_latched = 0;
                        hotkey_observed_mask = 0;
                        hotkey_stable_polls = 0;
                        hotkey_active_action = RUNTIME_ACTION_NONE;
                        hotkey_active_trigger = 0;
                        hotkey_hold_polls = 0;
                        hotkey_hold_last_seconds = -1;
                        hotkey_release_armed_mask = 0;
                        hotkey_release_armed_action =
                            RUNTIME_ACTION_NONE;
                        hotkey_desktop_session = 0;
                        debug_hotkey_settle_target = 0;
                    }

                } else if (hk != hotkey_observed_mask) {
                    unsigned int old_mask = hotkey_observed_mask;
                    const ps2vnc_hotkey_binding_t *new_release = NULL;
                    int release_action = RUNTIME_ACTION_NONE;
                    int pure_growth =
                        ((old_mask & hk) == old_mask);
                    int pure_shrink =
                        ((hk & old_mask) == hk);

                    /*
                     * Record where a new hotkey session began.  A button
                     * pressed while the OSK/menu owns input must never turn
                     * into a desktop shortcut merely because that context
                     * changes before the button is released.
                     */
                    if (hotkey_observed_mask == 0 && hk != 0)
                        hotkey_desktop_session =
                            desktop_hotkey_context;

                    /*
                     * An armed RELEASE binding fires only when its exact mask
                     * is broken by releasing buttons. Growing the gesture
                     * cancels it instead, preventing subset-action leakage.
                     */
                    if (hotkey_release_armed_action !=
                            RUNTIME_ACTION_NONE) {
                        if (hotkey_release_armed_mask == old_mask &&
                            pure_shrink &&
                            desktop_hotkey_context &&
                            hotkey_desktop_session) {
                            release_action =
                                hotkey_release_armed_action;
                        }

                        hotkey_release_armed_mask = 0;
                        hotkey_release_armed_action =
                            RUNTIME_ACTION_NONE;
                    }

                    hotkey_observed_mask = hk;
                    hotkey_stable_polls = 0;
                    hotkey_active_action = RUNTIME_ACTION_NONE;
                    hotkey_active_trigger = 0;
                    hotkey_hold_polls = 0;
                    hotkey_hold_last_seconds = -1;
                    debug_hotkey_settle_target = 0;

                    /*
                     * Changing a pending hold chord cancels its countdown,
                     * but deliberately leaves the System menu visible.
                     */
                    if (system_menu_countdown_action !=
                            RUNTIME_ACTION_NONE) {
                        system_menu_countdown_action =
                            RUNTIME_ACTION_NONE;
                        system_menu_countdown_seconds = 0;
                        osk_generation++;
                    }

                    /*
                     * Arm RELEASE only when arriving at its exact mask by
                     * adding buttons. Shrinking from a larger chord cannot
                     * manufacture a release shortcut.
                     */
                    if (pure_growth && hk != 0) {
                        new_release =
                            runtime_hotkey_lookup(
                                hk,
                                desktop_hotkey_context &&
                                hotkey_desktop_session);

                        if (new_release != NULL &&
                            new_release->trigger ==
                                HOTKEY_TRIGGER_RELEASE) {
                            hotkey_release_armed_mask = hk;
                            hotkey_release_armed_action =
                                new_release->action;
                        }
                    }

                    if (release_action != RUNTIME_ACTION_NONE) {
                        unsigned int keysym = 0;

                        if (release_action ==
                                RUNTIME_ACTION_SEND_TAB)
                            keysym = XK_TAB;
                        else if (release_action ==
                                 RUNTIME_ACTION_SEND_ENTER)
                            keysym = XK_RETURN;

                        if (keysym != 0 &&
                            send_key_tap(sock, keysym) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }

                        if (keysym != 0)
                            (void)wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y);
                    }

                    if (hk == 0)
                        hotkey_desktop_session = 0;

                } else if (hk != 0) {
                    const ps2vnc_hotkey_binding_t *binding;
                    unsigned int settle_polls =
                        RUNTIME_HOTKEY_SETTLE_POLLS;

                    hotkey_stable_polls++;

                    binding =
                        runtime_hotkey_lookup(
                            hk,
                            desktop_hotkey_context &&
                            hotkey_desktop_session);

                    /*
                     * Only ambiguous settle-actions get the longer collection
                     * window.  Maximal chords and HOLD bindings retain the
                     * normal short settle.
                     */
                    if (binding != NULL &&
                        binding->trigger == HOTKEY_TRIGGER_SETTLE &&
                        runtime_hotkey_has_hold_extension(
                            hk,
                            desktop_hotkey_context &&
                            hotkey_desktop_session)) {

                        settle_polls =
                            RUNTIME_HOTKEY_EXTENDABLE_SETTLE_POLLS;
                    }

                    debug_hotkey_settle_target = settle_polls;

                    if (hotkey_active_action ==
                            RUNTIME_ACTION_NONE &&
                        binding != NULL &&
                        binding->trigger != HOTKEY_TRIGGER_RELEASE &&
                        hotkey_stable_polls >= settle_polls) {

                        hotkey_active_action = binding->action;
                        hotkey_active_trigger = binding->trigger;

                        if (binding->trigger ==
                                HOTKEY_TRIGGER_SETTLE) {

                            if (binding->action ==
                                    RUNTIME_ACTION_SEND_ENTER) {

                                if (send_key_tap(
                                        sock, XK_RETURN) < 0) {
                                    controller_thread_error = 1;
                                    ExitThread();
                                }

                                (void)wake_renderer_with_pointer_jiggle(
                                    sock, cursor_x, cursor_y);

                            } else if (binding->action ==
                                       RUNTIME_ACTION_DISPLAY_SETTINGS) {

                                system_menu_visible = 0;
                                system_menu_status =
                                    SYSTEM_MENU_STATUS_NONE;

                                display_menu_open_for_active(0);

                                if (last_button_mask != 0) {
                                    (void)send_pointer_event(
                                        sock, 0,
                                        (unsigned int)cursor_x,
                                        (unsigned int)cursor_y);
                                    last_button_mask = 0;
                                }

                                (void)wake_renderer_with_pointer_jiggle(
                                    sock, cursor_x, cursor_y);
                            }

                            hotkey_latched = 1;

                        } else if (binding->trigger ==
                                   HOTKEY_TRIGGER_HOLD) {

                            if (!ps2vnc_system_overlay_allowed()) {

                                hotkey_active_action =
                                    RUNTIME_ACTION_NONE;
                                hotkey_active_trigger = 0;
                                hotkey_latched = 1;

                            } else {

                                /*
                                 * GLOBAL OVERLAY:
                                 *
                                 * Do not clear Display Settings or the OSK.
                                 * Existing renderer/input priority makes
                                 * System temporarily own the foreground while
                                 * preserving the ordinary UI underneath.
                                 */
                                system_menu_visible = 1;

                                system_menu_status =
                                    SYSTEM_MENU_STATUS_NONE;

                                system_menu_selection =
                                    (binding->action ==
                                     RUNTIME_ACTION_EXIT_TO_SYSTEM_MENU)
                                    ? SYSTEM_MENU_EXIT
                                    : SYSTEM_MENU_REFRESH;

                                system_menu_countdown_action =
                                    binding->action;
                                system_menu_countdown_seconds = 2;
                                hotkey_hold_polls = 0;
                                hotkey_hold_last_seconds = -1;
                                osk_generation++;

                                (void)wake_renderer_with_pointer_jiggle(
                                    sock, cursor_x, cursor_y);
                            }
                        }
                    }

                    if (hotkey_active_trigger ==
                            HOTKEY_TRIGGER_HOLD) {
                        unsigned int remaining;

                        hotkey_hold_polls++;

                        remaining =
                            (SYSTEM_CHORD_HOLD_POLLS >
                             hotkey_hold_polls)
                            ? ((SYSTEM_CHORD_HOLD_POLLS -
                                hotkey_hold_polls + 59u) / 60u)
                            : 0u;

                        if ((int)remaining !=
                                hotkey_hold_last_seconds) {
                            hotkey_hold_last_seconds = (int)remaining;
                            system_menu_countdown_seconds = remaining;
                            osk_generation++;
                        }

                        if (hotkey_hold_polls >=
                                SYSTEM_CHORD_HOLD_POLLS) {

                            system_menu_countdown_action =
                                RUNTIME_ACTION_NONE;
                            system_menu_countdown_seconds = 0;
                            osk_generation++;
                            hotkey_latched = 1;

                            if (hotkey_active_action ==
                                    RUNTIME_ACTION_REFRESH_RFB) {

                                (void)
                                    ps2vnc_system_refresh_accept_request();

                            } else if (hotkey_active_action ==
                                       RUNTIME_ACTION_EXIT_TO_SYSTEM_MENU) {

                                if (ps2vnc_system_exit_accept_request()) {

                                    debug_hotkey_mask =
                                        hotkey_observed_mask;
                                    debug_hotkey_stable_polls =
                                        hotkey_stable_polls;
                                    debug_hotkey_action =
                                        hotkey_active_action;
                                    debug_hotkey_trigger =
                                        hotkey_active_trigger;
                                    debug_hotkey_hold_polls =
                                        hotkey_hold_polls;
                                    debug_hotkey_latched =
                                        hotkey_latched;
                                    debug_hotkey_quarantine =
                                        local_ui_quarantine;

                                    ExitThread();
                                    return;
                                }
                            }
                        }
                    }
                }
            }

            debug_hotkey_mask = hotkey_observed_mask;
            debug_hotkey_stable_polls = hotkey_stable_polls;
            debug_hotkey_action = hotkey_active_action;
            debug_hotkey_trigger = hotkey_active_trigger;
            debug_hotkey_hold_polls = hotkey_hold_polls;
            debug_hotkey_latched = hotkey_latched;
            debug_hotkey_quarantine = local_ui_quarantine;

            /*
             * Display confirmation owns a real wall-clock deadline.
             * It starts before the hazardous switch; while the candidate
             * switch is still incomplete the UI remains hidden, but timeout
             * can already publish ROLLBACK and interrupt recv_exact().
             */
            if (display_confirm_pending &&
                display_confirm_action == DISPLAY_CONFIRM_ACTION_NONE &&
                (remote_display_state != REMOTE_DISPLAY_ACTIVE_TEMP ||
                 display_transition_switch_in_progress)) {

                unsigned int remaining =
                    display_confirm_remaining_seconds();

                if (remaining != display_confirm_seconds) {
                    display_confirm_seconds = remaining;

                    if (display_confirm_visible) {
                        osk_generation++;
                        (void)wake_renderer_with_pointer_jiggle(
                            sock, cursor_x, cursor_y);
                    }
                }

                if (remaining == 0 &&
                    display_confirm_pending &&
                    display_confirm_action ==
                        DISPLAY_CONFIRM_ACTION_NONE) {

                    display_confirm_action =
                        DISPLAY_CONFIRM_ACTION_ROLLBACK;
                    display_confirm_visible = 0;
                    osk_generation++;

                    /*
                     * H4C3D16 hard-stall fail-safe.
                     *
                     * Normally the main thread consumes ROLLBACK and restores
                     * A in-process.  If it is still trapped inside the risky
                     * synchronous mode reconstruction, it cannot consume that
                     * request.  Do not touch GS or RFB from this controller
                     * thread; replace the wedged process through the same
                     * established OSDSYS path used by main().
                     *
                     * The durable transaction intentionally remains
                     * provisional.  Existing startup reconciliation therefore
                     * makes the preserved previous profile authoritative.
                     */
                    if (display_transition_switch_in_progress)
                        LoadExecPS2("rom0:OSDSYS", 0, NULL);
                }
            }

            if (display_restore_ack_visible) {
                int ack_changed = 0;

                /*
                 * The restored acknowledgement owns these local UI buttons.
                 * Nothing may leak through to VNC or the menus underneath.
                 */
                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT |
                     PAD_CIRCLE | PAD_CROSS);

                if (!display_restore_ack_request &&
                    (changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {

                    display_restore_ack_request = 1;
                    ack_changed = 1;
                }

                if (ack_changed) {
                    (void)wake_renderer_with_pointer_jiggle(
                        sock, cursor_x, cursor_y);
                }

            } else /*
         * D17AL-F8J2-B4A:
         * Remote transitions now take true libpad ownership at the
         * transition boundary. Do not quarantine ordinary desktop
         * controller input for the complete ACTIVE_TEMP dwell.
         */
        if (display_confirm_pending) {
                int menu_changed = 0;

                /*
                 * F8J2-B3C:
                 *
                 * Remote provisional display control owns the controller
                 * through the SAME decision-tree boundary as the proven local
                 * display-confirmation transaction.
                 *
                 * A non-IDLE remote transaction therefore cannot fall through
                 * to desktop pointer/key/scroll generation.  Physical pad
                 * polling and the existing local/global hotkey machinery stay
                 * alive exactly as they do for the local confirmation UI.
                 *
                 * Human KEEP / GO BACK actions remain local-only below.
                 */

                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT |
                     PAD_CIRCLE | PAD_CROSS);

                if (remote_display_state == REMOTE_DISPLAY_IDLE &&
                    display_confirm_action ==
                        DISPLAY_CONFIRM_ACTION_NONE &&
                    display_confirm_visible &&
                    (((changed & PAD_UP) && (pressed & PAD_UP)) ||
                    ((changed & PAD_DOWN) && (pressed & PAD_DOWN)) ||
                    ((changed & PAD_LEFT) && (pressed & PAD_LEFT)) ||
                    ((changed & PAD_RIGHT) && (pressed & PAD_RIGHT)))) {

                    display_confirm_selection =
                        (display_confirm_selection ==
                         DISPLAY_CONFIRM_GO_BACK)
                        ? DISPLAY_CONFIRM_KEEP
                        : DISPLAY_CONFIRM_GO_BACK;

                    osk_generation++;
                    menu_changed = 1;
                }

                if (remote_display_state == REMOTE_DISPLAY_IDLE &&
                    display_confirm_action ==
                        DISPLAY_CONFIRM_ACTION_NONE &&
                    (changed & PAD_CIRCLE) &&
                    (pressed & PAD_CIRCLE)) {

                    display_confirm_action =
                        DISPLAY_CONFIRM_ACTION_ROLLBACK;
                    display_confirm_visible = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if (remote_display_state == REMOTE_DISPLAY_IDLE &&
                    display_confirm_action ==
                        DISPLAY_CONFIRM_ACTION_NONE &&
                    (changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {

                    display_confirm_action =
                        (display_confirm_selection ==
                         DISPLAY_CONFIRM_KEEP)
                        ? DISPLAY_CONFIRM_ACTION_KEEP
                        : DISPLAY_CONFIRM_ACTION_ROLLBACK;

                    display_confirm_visible = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if (menu_changed) {
                    (void)wake_renderer_with_pointer_jiggle(
                        sock, cursor_x, cursor_y);
                }

            } else if (system_menu_visible) {
                int menu_changed = 0;

                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT |
                     PAD_CIRCLE | PAD_CROSS);

                if (((changed & PAD_UP) && (pressed & PAD_UP)) ||
                    ((changed & PAD_LEFT) && (pressed & PAD_LEFT))) {

                    system_menu_selection =
                        (system_menu_selection +
                         SYSTEM_MENU_ITEM_COUNT - 1) %
                        SYSTEM_MENU_ITEM_COUNT;

                    system_menu_status =
                        SYSTEM_MENU_STATUS_NONE;
                    system_menu_countdown_action =
                        RUNTIME_ACTION_NONE;
                    system_menu_countdown_seconds = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if (((changed & PAD_DOWN) && (pressed & PAD_DOWN)) ||
                    ((changed & PAD_RIGHT) && (pressed & PAD_RIGHT))) {

                    system_menu_selection =
                        (system_menu_selection + 1) %
                        SYSTEM_MENU_ITEM_COUNT;

                    system_menu_status =
                        SYSTEM_MENU_STATUS_NONE;
                    system_menu_countdown_action =
                        RUNTIME_ACTION_NONE;
                    system_menu_countdown_seconds = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_CIRCLE) &&
                    (pressed & PAD_CIRCLE)) {

                    system_menu_visible = 0;
                    system_menu_status =
                        SYSTEM_MENU_STATUS_NONE;
                    system_menu_countdown_action =
                        RUNTIME_ACTION_NONE;
                    system_menu_countdown_seconds = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {

                    system_menu_countdown_action =
                        RUNTIME_ACTION_NONE;
                    system_menu_countdown_seconds = 0;

                    if (system_menu_selection ==
                        SYSTEM_MENU_REFRESH) {

                        (void)
                            ps2vnc_system_refresh_accept_request();

                    } else if (system_menu_selection ==
                               SYSTEM_MENU_DISPLAY) {

                        system_menu_visible = 0;
                        system_menu_status =
                            SYSTEM_MENU_STATUS_NONE;

                        if (display_menu_visible) {

                            /*
                             * Display Settings was already the preserved
                             * underlay. Reveal it exactly where it was.
                             */
                            osk_generation++;
                            menu_changed = 1;

                        } else {

                            /*
                             * Deliberate navigation from desktop/OSK into
                             * Display Settings. The previous OSK is not part
                             * of this new navigation branch.
                             */
                            osk_visible = 0;
                            display_menu_open_for_active(1);
                            menu_changed = 1;
                        }

                    } else {

                        if (ps2vnc_system_exit_accept_request()) {
                            ExitThread();
                            return;
                        }

                        osk_generation++;
                        menu_changed = 1;
                    }
                }

                if (menu_changed) {
                    /*
                     * Local overlay wake is best-effort. Never sacrifice the
                     * controller thread merely because the VNC stream is bad.
                     */
                    (void)wake_renderer_with_pointer_jiggle(
                        sock, cursor_x, cursor_y);
                }


            } else if (display_menu_visible) {

                int menu_changed = 0;

                /*
                 * Square is now owned locally while Display Settings is open.
                 * Outside this menu it remains the desktop Backspace shortcut.
                 */
                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_UP | PAD_DOWN |
                     PAD_LEFT | PAD_RIGHT |
                     PAD_CIRCLE | PAD_CROSS |
                     PAD_SQUARE);

                if ((changed & PAD_UP) &&
                    (pressed & PAD_UP)) {

                    display_menu_selection =
                        display_menu_move_vertical_selection(
                            display_menu_selection,
                            -1);

                    display_menu_error = 0;
                    display_policy_error = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_DOWN) &&
                    (pressed & PAD_DOWN)) {

                    display_menu_selection =
                        display_menu_move_vertical_selection(
                            display_menu_selection,
                            1);

                    display_menu_error = 0;
                    display_policy_error = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_LEFT) &&
                    (pressed & PAD_LEFT)) {

                    display_menu_selection =
                        display_menu_move_horizontal_selection(
                            display_menu_selection,
                            -1);

                    display_menu_error = 0;
                    display_policy_error = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_RIGHT) &&
                    (pressed & PAD_RIGHT)) {

                    display_menu_selection =
                        display_menu_move_horizontal_selection(
                            display_menu_selection,
                            1);

                    display_menu_error = 0;
                    display_policy_error = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_CIRCLE) &&
                    (pressed & PAD_CIRCLE)) {

                    display_menu_visible = 0;
                    display_menu_error = 0;
                    display_policy_error = 0;

                    if (display_menu_parent_system) {
                        display_menu_parent_system = 0;
                        system_menu_selection =
                            SYSTEM_MENU_DISPLAY;
                        system_menu_visible = 1;
                    }

                    osk_generation++;
                    menu_changed = 1;
                }

                /*
                 * Square toggles per-mode admission policy.
                 *
                 * The current active mode is deliberately not lockable from
                 * the UI. Unlocking does not switch into the mode.
                 */
                if ((changed & PAD_SQUARE) &&
                    (pressed & PAD_SQUARE) &&
                    display_menu_tab !=
                        DISPLAY_TAB_ADVANCED) {

                    const ps2vnc_video_mode_t *selected_mode =
                        display_menu_mode_at_selection(
                            display_menu_selection);

                    if (selected_mode != NULL &&
                        selected_mode != active_video_mode &&
                        display_policy_loaded &&
                        display_policy_request ==
                            DISPLAY_POLICY_REQUEST_NONE) {

                        display_policy_error = 0;

                        display_policy_request_selection =
                            display_menu_selection;

                        display_policy_request_value =
                            display_mode_locked[
                                display_menu_selection]
                                ? 0
                                : 1;

                        display_policy_request =
                            DISPLAY_POLICY_REQUEST_LOCK;

                        osk_generation++;
                        menu_changed = 1;
                    }
                }

                if ((changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {

                    /*
                     * ADVANCED currently contains one setting row.
                     */
                    if (display_menu_tab ==
                            DISPLAY_TAB_ADVANCED) {

                        if (display_policy_loaded &&
                            display_policy_request ==
                                DISPLAY_POLICY_REQUEST_NONE) {

                            display_policy_error = 0;

                            display_policy_request_value =
                                display_hide_locked_modes
                                    ? 0
                                    : 1;

                            display_policy_request =
                                DISPLAY_POLICY_REQUEST_HIDE_LOCKED;

                            osk_generation++;
                            menu_changed = 1;
                        }

                    } else {

                        const ps2vnc_video_mode_t *selected_mode =
                            display_menu_mode_at_selection(
                                display_menu_selection);

                        if (selected_mode == NULL) {
                            /*
                             * Empty filtered tab: hard no-op.
                             */

                        } else if (selected_mode ==
                                      active_video_mode) {

                            display_menu_visible = 0;
                            display_menu_error = 0;
                            display_policy_error = 0;
                            display_menu_parent_system = 0;

                            display_calibration_request =
                                display_mode_request_for_selection(
                                    display_menu_selection);

                            osk_generation++;
                            menu_changed = 1;

                        } else if (!display_policy_loaded) {

                            /*
                             * Never switch into an inactive mode without
                             * knowing authoritative lock policy.
                             */
                            display_policy_error = 1;
                            osk_generation++;
                            menu_changed = 1;

                        } else if (display_mode_locked[
                                      display_menu_selection]) {

                            /*
                             * HARD NO-OP:
                             *
                             * No transaction request, no management resize,
                             * no GS operation, no startup persistence.
                             */
                            osk_generation++;
                            menu_changed = 1;

                        } else {

                            display_menu_visible = 0;
                            display_menu_error = 0;
                            display_policy_error = 0;
                            display_menu_parent_system = 0;

                            display_mode_request_remote = 0;
                            display_mode_request =
                                display_mode_request_for_selection(
                                    display_menu_selection);

                            osk_generation++;
                            menu_changed = 1;
                        }
                    }
                }

                if (menu_changed) {
                    if (wake_renderer_with_pointer_jiggle(
                            sock,
                            cursor_x,
                            cursor_y) < 0) {

                        controller_thread_error = 1;
                        ExitThread();
                    }
                }

            } else if (!osk_visible) {

                unsigned int desktop_pressed =
                    pressed & ~local_ui_quarantine;
                unsigned int desktop_changed =
                    desktop_pressed ^
                    (last_pressed & ~local_ui_quarantine);

                /*
                 * Open the OSK with Select.  Reset to Q for predictable
                 * entry and ensure no mouse button remains logically held.
                 */
                if ((desktop_changed & PAD_SELECT) &&
                    (desktop_pressed & PAD_SELECT)) {

                    osk_visible = 1;
                    osk_page = OSK_PAGE_ABC;
                    osk_row = 1;
                    osk_col = 0;
                    osk_shift = 0;
                    osk_ctrl = 0;
                    osk_alt = 0;
                    osk_generation++;

                    if (last_button_mask != 0) {
                        if (send_pointer_event(
                                sock, 0,
                                (unsigned int)cursor_x,
                                (unsigned int)cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        last_button_mask = 0;
                    }

                    if (wake_renderer_with_pointer_jiggle(
                            sock, cursor_x, cursor_y) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }

                    /*
                     * last_pressed always tracks the raw physical pad state.
                     * Quarantine affects consumers, never edge bookkeeping.
                     */
                    last_pressed = pressed;
                    nanosleep(&poll_delay, NULL);
                    continue;
                }

                /*
                 * Quick keyboard shortcuts remain available while using
                 * normal mouse mode.
                 */
                if ((desktop_changed & PAD_SQUARE) &&
                    (desktop_pressed & PAD_SQUARE)) {
                    if (send_key_tap(sock, XK_BACKSPACE) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    if (wake_renderer_with_pointer_jiggle(
                            sock, cursor_x, cursor_y) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                }

                /*
                 * R1=Tab and L1+Start=Enter are resolved by the generic
                 * runtime-hotkey arbiter above.
                 */

                /*
                 * TEST13I:
                 * Triangle + D-pad = mouse-wheel scrolling.
                 *
                 * This takes priority over L1+D-pad and ordinary D-pad
                 * precision mouse movement. The OSK has its own input branch,
                 * so Triangle continues to mean Shift while the OSK is open.
                 *
                 * Vertical directions take priority over horizontal directions
                 * if a diagonal D-pad combination is reported.
                 */
                if (desktop_pressed & PAD_TRIANGLE) {
                    unsigned int dpad =
                        desktop_pressed &
                        (PAD_LEFT | PAD_RIGHT |
                         PAD_UP | PAD_DOWN);

                    /*
                     * Do not preserve any fractional/hold state from ordinary
                     * D-pad mouse movement while Triangle owns the D-pad.
                     */
                    dpad_hold_direction = 0;
                    dpad_hold_ticks = 0;
                    dpad_x_q8 = 0;
                    dpad_y_q8 = 0;

                    if ((dpad & PAD_UP) &&
                        !(dpad & PAD_DOWN)) {
                        scroll_mask = RFB_SCROLL_UP;
                    } else if ((dpad & PAD_DOWN) &&
                               !(dpad & PAD_UP)) {
                        scroll_mask = RFB_SCROLL_DOWN;
                    } else if ((dpad & PAD_LEFT) &&
                               !(dpad & PAD_RIGHT)) {
                        scroll_mask = RFB_SCROLL_LEFT;
                    } else if ((dpad & PAD_RIGHT) &&
                               !(dpad & PAD_LEFT)) {
                        scroll_mask = RFB_SCROLL_RIGHT;
                    }

                    if (scroll_mask != 0)
                        scroll_delay = DPAD_SCROLL_REPEAT_POLLS;

                /*
                 * Test11F: L1 is a physical chord modifier, not a mode.
                 * No state is remembered and nothing can remain toggled.
                 */
                } else if (desktop_pressed & PAD_L1) {
                    if ((desktop_pressed & PAD_UP) &&
                        (desktop_changed & (PAD_L1 | PAD_UP))) {
                        if (send_key_tap(sock, XK_UP) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        if (wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                    }

                    if ((desktop_pressed & PAD_DOWN) &&
                        (desktop_changed & (PAD_L1 | PAD_DOWN))) {
                        if (send_key_tap(sock, XK_DOWN) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        if (wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                    }

                    if ((desktop_pressed & PAD_LEFT) &&
                        (desktop_changed & (PAD_L1 | PAD_LEFT))) {
                        if (send_key_tap(sock, XK_LEFT) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        if (wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                    }

                    if ((desktop_pressed & PAD_RIGHT) &&
                        (desktop_changed & (PAD_L1 | PAD_RIGHT))) {
                        if (send_key_tap(sock, XK_RIGHT) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        if (wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                    }
                } else {
                    /* Normal D-pad = precise mouse movement. */
                    {
                        unsigned int dpad =
                            desktop_pressed &
                            (PAD_LEFT | PAD_RIGHT |
                             PAD_UP | PAD_DOWN);

                        unsigned int dpad_new =
                            desktop_changed &
                            desktop_pressed &
                            (PAD_LEFT | PAD_RIGHT |
                             PAD_UP | PAD_DOWN);

                        int do_dpad_step = 0;

                        if (dpad_new != 0) {
                            /*
                             * Fresh tap: exactly two pixels.
                             */
                            dpad_hold_direction = dpad;
                            dpad_hold_ticks = 0;
                            dpad_x_q8 = 0;
                            dpad_y_q8 = 0;
                            do_dpad_step = 1;

                        } else if (dpad == 0) {
                            dpad_hold_direction = 0;
                            dpad_hold_ticks = 0;
                            dpad_x_q8 = 0;
                            dpad_y_q8 = 0;

                        } else if (dpad != dpad_hold_direction) {
                            /*
                             * Direction change behaves like a fresh tap.
                             */
                            dpad_hold_direction = dpad;
                            dpad_hold_ticks = 0;
                            dpad_x_q8 = 0;
                            dpad_y_q8 = 0;
                            do_dpad_step = 1;

                        } else {
                            dpad_hold_ticks++;

                            if (dpad_hold_ticks <
                                DPAD_ACCEL_START_TICKS) {

                                /*
                                 * Preserve 11K-F exactly during the
                                 * precision phase:
                                 *
                                 * ~300 ms delay, then 2 pixels every
                                 * four controller polls.
                                 */
                                if (dpad_hold_ticks >= 18 &&
                                    ((dpad_hold_ticks - 18) % 4) == 0)
                                    do_dpad_step = 1;

                            } else {
                                int velocity =
                                    dpad_hold_velocity_q8(
                                        dpad_hold_ticks);

                                /*
                                 * After one second, use fractional
                                 * continuous movement so velocity can
                                 * accelerate smoothly.
                                 */
                                if (dpad & PAD_LEFT)
                                    dpad_x_q8 -= velocity;

                                if (dpad & PAD_RIGHT)
                                    dpad_x_q8 += velocity;

                                if (dpad & PAD_UP)
                                    dpad_y_q8 -= velocity;

                                if (dpad & PAD_DOWN)
                                    dpad_y_q8 += velocity;

                                while (dpad_x_q8 >= 256) {
                                    dx++;
                                    dpad_x_q8 -= 256;
                                }

                                while (dpad_x_q8 <= -256) {
                                    dx--;
                                    dpad_x_q8 += 256;
                                }

                                while (dpad_y_q8 >= 256) {
                                    dy++;
                                    dpad_y_q8 -= 256;
                                }

                                while (dpad_y_q8 <= -256) {
                                    dy--;
                                    dpad_y_q8 += 256;
                                }
                            }
                        }

                        if (do_dpad_step) {
                            if (dpad & PAD_LEFT)  dx -= 2;
                            if (dpad & PAD_RIGHT) dx += 2;
                            if (dpad & PAD_UP)    dy -= 2;
                            if (dpad & PAD_DOWN)  dy += 2;
                        }
                    }
                }

                /*
                 * Left analog normally remains accelerated mouse movement.
                 *
                 * While L3 is held, however, the same stick becomes an
                 * analog mouse wheel and does NOT move the cursor.
                 */
                if ((buttons.mode & 0xF0) == 0x70) {
                    if (desktop_pressed & PAD_L3) {
                        /*
                         * L3 is a hard pointer freeze.
                         *
                         * Clear fractional mouse motion as soon as scrolling
                         * begins so releasing L3 cannot reveal an old partial
                         * cursor movement.
                         */
                        analog_x_q8 = 0;
                        analog_y_q8 = 0;

                        /*
                         * Triangle+D-pad has explicit priority. Only derive
                         * scrolling from the analog stick if no D-pad wheel
                         * direction was selected above.
                         */
                        if (scroll_mask == 0) {
                            scroll_mask = scroll_button_for_stick(
                                buttons.ljoy_h,
                                buttons.ljoy_v,
                                &scroll_magnitude
                            );

                            if (scroll_mask != 0) {
                                scroll_delay =
                                    scroll_repeat_delay(scroll_magnitude);
                            }
                        }
                    } else {
                        int vx =
                            stick_velocity_q8(buttons.ljoy_h);

                        int vy =
                            stick_velocity_q8(buttons.ljoy_v);

                        /*
                         * Returning an axis to center immediately clears its
                         * fractional remainder.
                         */
                        if (vx == 0)
                            analog_x_q8 = 0;
                        else
                            analog_x_q8 += vx;

                        if (vy == 0)
                            analog_y_q8 = 0;
                        else
                            analog_y_q8 += vy;

                        while (analog_x_q8 >= 256) {
                            dx++;
                            analog_x_q8 -= 256;
                        }

                        while (analog_x_q8 <= -256) {
                            dx--;
                            analog_x_q8 += 256;
                        }

                        while (analog_y_q8 >= 256) {
                            dy++;
                            analog_y_q8 -= 256;
                        }

                        while (analog_y_q8 <= -256) {
                            dy--;
                            analog_y_q8 += 256;
                        }
                    }
                }

                cursor_x += dx;
                cursor_y += dy;

                if (cursor_x < 0)
                    cursor_x = 0;
                if (cursor_x >= (int)desktop_width)
                    cursor_x = (int)desktop_width - 1;

                if (cursor_y < 0)
                    cursor_y = 0;
                if (cursor_y >= (int)desktop_height)
                    cursor_y = (int)desktop_height - 1;

                button_mask =
                    ((desktop_pressed & PAD_CROSS)  ? 0x01 : 0x00) |
                    ((desktop_pressed & PAD_CIRCLE) ? 0x04 : 0x00);

                if (cursor_x != old_x ||
                    cursor_y != old_y ||
                    button_mask != last_button_mask) {

                    if (send_pointer_event(
                            sock,
                            button_mask,
                            (unsigned int)cursor_x,
                            (unsigned int)cursor_y) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }

                    last_button_mask = button_mask;
                }

                /*
                 * Test11K scroll mode.
                 *
                 * Direction changes fire immediately. Holding the same
                 * direction repeats according to analog deflection.
                 */
                if (scroll_mask != 0) {
                    if (scroll_mask != last_scroll_mask)
                        scroll_repeat_countdown = 0;

                    if (scroll_repeat_countdown == 0) {
                        if (send_scroll_pulse(
                                sock,
                                button_mask,
                                scroll_mask,
                                cursor_x,
                                cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }

                        scroll_repeat_countdown = scroll_delay;
                    } else {
                        scroll_repeat_countdown--;
                    }

                    last_scroll_mask = scroll_mask;
                } else {
                    last_scroll_mask = 0;
                    scroll_repeat_countdown = 0;
                }
            } else {
                int osk_changed = 0;

                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_LEFT | PAD_RIGHT | PAD_UP | PAD_DOWN |
                     PAD_TRIANGLE | PAD_CROSS | PAD_SQUARE |
                     PAD_START | PAD_R1 | PAD_CIRCLE | PAD_SELECT);

                /*
                 * While the OSK is open the D-pad controls the keyboard,
                 * not the remote mouse.  Movement is edge-triggered for
                 * this first version.
                 */
                if ((changed & PAD_LEFT) &&
                    (pressed & PAD_LEFT)) {
                    osk_move_horizontal(-1);
                    osk_changed = 1;
                }

                if ((changed & PAD_RIGHT) &&
                    (pressed & PAD_RIGHT)) {
                    osk_move_horizontal(1);
                    osk_changed = 1;
                }

                if ((changed & PAD_UP) &&
                    (pressed & PAD_UP)) {
                    osk_move_vertical(-1);
                    osk_changed = 1;
                }

                if ((changed & PAD_DOWN) &&
                    (pressed & PAD_DOWN)) {
                    osk_move_vertical(1);
                    osk_changed = 1;
                }

                if ((changed & PAD_TRIANGLE) &&
                    (pressed & PAD_TRIANGLE)) {
                    osk_shift = !osk_shift;
                    osk_generation++;
                    osk_changed = 1;
                }

                if ((changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {
                    if (osk_activate_selected(sock) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    osk_changed = 1;
                }

                if ((changed & PAD_SQUARE) &&
                    (pressed & PAD_SQUARE)) {
                    if (osk_send_key_with_modifiers(
                            sock, XK_BACKSPACE) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    osk_changed = 1;
                }

                if ((changed & PAD_START) &&
                    (pressed & PAD_START)) {
                    if (osk_send_key_with_modifiers(
                            sock, XK_RETURN) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    osk_changed = 1;
                }

                if ((changed & PAD_R1) &&
                    (pressed & PAD_R1)) {
                    if (osk_send_key_with_modifiers(
                            sock, XK_TAB) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    osk_changed = 1;
                }

                if (((changed & PAD_CIRCLE) &&
                     (pressed & PAD_CIRCLE)) ||
                    ((changed & PAD_SELECT) &&
                     (pressed & PAD_SELECT))) {

                    osk_visible = 0;
                    osk_shift = 0;
                    osk_ctrl = 0;
                    osk_alt = 0;
                    osk_generation++;
                    osk_changed = 1;
                }

                /*
                 * Purely local keyboard changes need to wake the main
                 * renderer, which may be blocked in an incremental VNC
                 * request.  Actual typed keys normally cause a remote
                 * redraw themselves.
                 */
                if (osk_changed) {
                    if (wake_renderer_with_pointer_jiggle(
                            sock, cursor_x, cursor_y) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                }
            }

            last_pressed = pressed;
        }

        nanosleep(&poll_delay, NULL);
    }
}

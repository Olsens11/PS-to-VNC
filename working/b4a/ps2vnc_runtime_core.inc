/* M4G2: video-mode lookup/default logic lives in src/video/mode.c. */

/* Built-in fallback when no valid persisted startup mode exists. */
const ps2vnc_video_mode_t *active_video_mode = &video_mode_1080i;
ps2vnc_backend_t active_display_backend = PS2VNC_BACKEND_NONE;


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
unsigned int ps2vnc_hires_effective_width(void)
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


 int ps2vnc_hires_requires_vertical_mapping(void)
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


int ps2vnc_hires_requires_raster_mapping(void)
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
unsigned int ps2vnc_hires_storage_width(void)
{
    return ps2vnc_hires_effective_width();
}


unsigned int ps2vnc_hires_storage_height(void)
{
    if (active_video_mode == NULL)
        return 0;

    if (ps2vnc_hires_requires_vertical_mapping())
        return active_video_mode->hires_height;

    return active_video_mode->raster_height;
}


unsigned int ps2vnc_hires_storage_bytes(void)
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
 int ps2vnc_display_logical_desktop_size(
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

float ps2vnc_display_output_x(float x)
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

float ps2vnc_display_output_y(float y)
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
unsigned int desktop_width = VNC_WIDTH;
unsigned int desktop_height = VNC_HEIGHT;

/*
 * Position of the 1:1 remote desktop inside the fixed 1280x720
 * physical HIRES output.
 */
unsigned int desktop_output_x = 0;
unsigned int desktop_output_y = 0;
unsigned int desktop_output_width = VNC_WIDTH;
unsigned int desktop_output_height = VNC_HEIGHT;

unsigned short *hires_bg_buffer = NULL;
unsigned short *hires_bg_back_buffer = NULL;
unsigned short *live_direct_present_buffer = NULL;

/*
 * D17AL-F7B:
 *
 * For the isolated 480p native-surface path, Hextile continues decoding into
 * the ordinary authoritative CPU framebuffer. The presentation backend then
 * maps that complete image into 704x462.
 */
unsigned short *hires_mapped_source_framebuffer = NULL;
int hires_bg_configured = 0;
int live_direct_present_safe = 0;
int live_linear_framebuffer_stale = 0;
static unsigned int hires_bg_storage_row(unsigned int raster_y);

/*
 * Before calibration we accept whatever <=1280x720 geometry TigerVNC
 * currently has. Once the safe desktop is finalized, recovery sessions
 * will be required to return at exactly that geometry.
 */
int desktop_geometry_locked = 0;
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
 unsigned char pad_area[256] __attribute__((aligned(64)));

/* Dedicated EE input thread. */
#define CONTROLLER_STACK_SIZE 8192
 unsigned char controller_stack[CONTROLLER_STACK_SIZE]
    __attribute__((aligned(16)));

 int controller_thread_id = -1;
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
 volatile unsigned int rfb_out_head = 0;
 volatile unsigned int rfb_out_tail = 0;
 volatile unsigned int rfb_out_count = 0;
int rfb_queue_sema = -1;

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
 unsigned int rfb_rx_pos = 0;
 unsigned int rfb_rx_end = 0;
 volatile unsigned int rfb_rx_syscalls = 0;

/*
 * Test31 receive/header sentinels retained for lightweight diagnostics.
 * The RX buffer is prefilled with A5 and rectangle headers with 5A so a
 * short/incomplete copy remains visible in the normal debug state.
 */
#define TEST31_RX_SENTINEL 0xA5
#define TEST31_HDR_SENTINEL 0x5A

 volatile int rfb_last_recv_n = 0;
 volatile unsigned int rfb_last_recv_call = 0;

 volatile unsigned int live_hdr_start_pos = 0;
 volatile unsigned int live_hdr_start_end = 0;
 volatile unsigned int live_hdr_start_avail = 0;
 volatile unsigned int live_hdr_end_pos = 0;
 volatile unsigned int live_hdr_end_end = 0;
 volatile int live_hdr_recv_n = 0;
 volatile unsigned int live_hdr_recv_call = 0;
 volatile unsigned char live_hdr_raw[12];

/* Raw FramebufferUpdate header tracing: type + pad + rectangle count. */
 volatile unsigned char live_update_raw[4];
 volatile unsigned int live_update_start_pos = 0;
 volatile unsigned int live_update_start_end = 0;
 volatile unsigned int live_update_end_pos = 0;
 volatile unsigned int live_update_end_end = 0;
 volatile unsigned int live_update_recv_call = 0;



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
 unsigned char test29_row_scratch[VNC_WIDTH * 2];

/*
 * TEST13F:
 * Bounding box of all Raw rectangles in the most recently completed
 * RFB FramebufferUpdate. x1/y1 are exclusive.
 */
static unsigned int rfb_dirty_x0 = 0;
 unsigned int rfb_dirty_y0 = 0;
static unsigned int rfb_dirty_x1 = 0;
 unsigned int rfb_dirty_y1 = 0;
 int rfb_dirty_valid = 0;

/*
 * TEST15D3-A3-A3:
 * Keep a conservative dirty X span for each desktop row in addition to the
 * historical whole-update bounding box.  Multiple RFB rectangles touching
 * the same row merge naturally.
 */
 unsigned int rfb_dirty_row_x0[VNC_HEIGHT];
 unsigned int rfb_dirty_row_x1[VNC_HEIGHT];
 unsigned char rfb_dirty_row_valid[VNC_HEIGHT];

 void rfb_dirty_reset(void)
{
    rfb_dirty_valid = 0;
    memset(
        rfb_dirty_row_valid,
        0,
        sizeof(rfb_dirty_row_valid)
    );
}

 void rfb_dirty_mark_rect(
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
volatile int controller_pad_pause_requested = 0;
volatile int controller_pad_pause_ack = 0;

volatile int controller_thread_error = 0;

/*
 * TEST12B CONNECTION RECOVERY
 *
 * Controller input keeps running during a Pi/VNC outage, but normal
 * RFB input events are discarded until a complete replacement RFB
 * session is established.
 */
#define RECONNECT_TIMEOUT_SECONDS 10
#define RECONNECT_MAX_ATTEMPTS    10

volatile int rfb_connected = 0;
volatile int exit_requested = 0;

/*
 * D4 startup override:
 * hold L1 + R1 while PS2VNC starts to force interactive display
 * calibration even when a valid stored display preset was loaded.
 */
int force_screen_fit_calibration = 0;

 int controller_sock = -1;

/* Last live RFB failure context, displayed if the render loop exits. */
 volatile int live_error_code = 0;
 volatile unsigned int live_message_type = 0;
 volatile unsigned int live_rect_count = 0;
 volatile unsigned int live_rect_index = 0;
 volatile unsigned int live_rect_x = 0;
 volatile unsigned int live_rect_y = 0;
 volatile unsigned int live_rect_w = 0;
 volatile unsigned int live_rect_h = 0;
 volatile unsigned int live_encoding = 0;

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
 volatile unsigned int debug_frame_counter = 0;
 volatile int debug_changed_bytes = 0;

/* D17AL-F8J2-B4A hard-transition envelope phase, 0 = idle. */
 volatile unsigned int debug_transition_phase = 0;

/* TEST15C1 measurement-only live-pipeline profiler. */
 u64 profile_present_sync_ticks = 0;
 u64 profile_flip_ticks = 0;
 u64 profile_rfb_ticks = 0;
u64 profile_rx_wait_ticks = 0;
 u64 profile_hextile_ticks = 0;
u64 profile_hextile_rx_wait_ticks = 0;
 int profile_hextile_active = 0;
 u64 profile_dirty_wait_ticks = 0;
 u64 profile_dirty_work_ticks = 0;
 u64 profile_rfb_start_tick = 0;
 u64 profile_dirty_work_start_tick = 0;
 unsigned int profile_rfb_updates = 0;
 unsigned int profile_dirty_updates = 0;
 unsigned int profile_changed_bytes = 0;
 unsigned int profile_raw_rects = 0;
 unsigned int profile_hextile_rects = 0;
 volatile unsigned int controller_loop_counter = 0;
unsigned int debug_idle_poll_counter = 0;

/*
 * TEST15E3H controller-state mirror.
 *
 * The controller thread writes only these scalar diagnostics.  UDP remains
 * owned exclusively by the main/RFB thread through debug_emit_now().
 */
 volatile unsigned int debug_hotkey_mask = 0;
 volatile unsigned int debug_hotkey_stable_polls = 0;
 volatile unsigned int debug_hotkey_settle_target = 0;
/* Runtime action NONE is numerically zero; avoid enum declaration order. */
 volatile int debug_hotkey_action = 0;
 volatile int debug_hotkey_trigger = 0;
 volatile unsigned int debug_hotkey_hold_polls = 0;
 volatile int debug_hotkey_latched = 0;
 volatile unsigned int debug_hotkey_quarantine = 0;

/* On-screen keyboard state shared between controller and renderer. */
 volatile int osk_visible = 0;
 volatile int osk_row = 1;
 volatile int osk_col = 0;
 volatile int osk_shift = 0;
 volatile int osk_ctrl = 0;
 volatile int osk_alt = 0;
 volatile int osk_page = 0;
volatile unsigned int osk_generation = 1;

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

int display_menu_selection_for_mode(
    const ps2vnc_video_mode_t *mode)
{
    unsigned int i;

    for (i = 0; i < VIDEO_MODE_COUNT; i++) {
        if (video_modes[i] == mode)
            return (int)i;
    }

    return 0;
}

const ps2vnc_video_mode_t *display_menu_mode_at_selection(
    int selection)
{
    if (selection < 0 ||
        (unsigned int)selection >= VIDEO_MODE_COUNT)
        return NULL;

    return video_modes[selection];
}

 int display_mode_request_for_selection(int selection)
{
    if (selection < 0 ||
        (unsigned int)selection >= VIDEO_MODE_COUNT)
        return DISPLAY_MODE_REQUEST_NONE;

    return selection + 1;
}

 const ps2vnc_video_mode_t *display_mode_request_target(
    int request)
{
    int selection = request - 1;

    return display_menu_mode_at_selection(selection);
}

 volatile int display_menu_visible = 0;
 volatile int display_menu_selection = 0;
 volatile int display_menu_error = 0;
 volatile int display_mode_request = DISPLAY_MODE_REQUEST_NONE;

/*
 * A calibration request is semantically distinct from a mode switch even
 * though both carry the registry selection as index+1.  Same-mode X publishes
 * only this channel; it must never enter ps2vnc_switch_display_mode().
 */

 volatile int display_calibration_request =
    DISPLAY_MODE_REQUEST_NONE;


enum {
    REMOTE_DISPLAY_IDLE = 0,
    REMOTE_DISPLAY_SWITCH_REQUESTED,
    REMOTE_DISPLAY_SWITCHING,
    REMOTE_DISPLAY_ACTIVE_TEMP,
    REMOTE_DISPLAY_RESTORING
};

 volatile int remote_display_state =
    REMOTE_DISPLAY_IDLE;

/*
 * F8J1C:
 * Successful remote rollback is machine-owned and must return to the
 * ordinary UI underlay.  Legacy human transaction callers still invoke
 * display_confirm_return_to_active_menu(); consume that call once instead
 * of duplicating remote special-cases at every rollback exit.
 */
 volatile int
    remote_display_suppress_next_active_menu_return = 0;

 volatile int display_mode_request_remote = 0;

 unsigned int remote_display_txid = 0;
 unsigned int remote_display_lease_seconds = 0;
 unsigned int remote_display_error_code = 0;

/* F8J2-B2 diagnostic: exact failure-return ordinal inside
 * ps2vnc_switch_display_mode(). */
 volatile unsigned int display_switch_failure_stage = 0;

 u64 remote_display_lease_started_tick = 0;
 u64 remote_display_last_poll_tick = 0;

 const ps2vnc_video_mode_t *
    remote_display_requested_mode = NULL;

 const ps2vnc_video_mode_t *
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

 volatile int display_menu_tab = DISPLAY_TAB_TV_DTV;

/*
 * Remember the most recent registry selection in each mode tab.
 * ADVANCED has settings rows rather than a mode selection.
 */
 volatile int display_menu_tab_selection[3] = {
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
volatile unsigned char display_mode_locked[VIDEO_MODE_COUNT];

volatile int display_hide_locked_modes = 0;
volatile int display_policy_loaded = 0;
 volatile int display_policy_error = 0;

/*
 * Controller -> main-thread semantic policy request.
 *
 * The controller never performs management HTTP.
 */
 volatile int display_policy_request =
    DISPLAY_POLICY_REQUEST_NONE;

 volatile int display_policy_request_selection = -1;
 volatile int display_policy_request_value = 0;

 void display_menu_open_for_active(int parent_system);

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

 volatile int display_confirm_pending = 0;
 volatile int display_confirm_visible = 0;
 volatile int display_confirm_selection = DISPLAY_CONFIRM_GO_BACK;
 volatile unsigned int display_confirm_seconds = 0;
 volatile int display_confirm_action = DISPLAY_CONFIRM_ACTION_NONE;

/*
 * H4C3D16:
 * True only while the main/render thread is inside the synchronous candidate
 * display reconstruction.  The controller thread already owns the independent
 * wall-clock confirmation countdown; this flag lets it distinguish an ordinary
 * confirmation timeout from a main-thread transition stall.
 */
 volatile int display_transition_switch_in_progress = 0;

 const ps2vnc_video_mode_t *display_confirm_previous_mode = NULL;
 unsigned int display_confirm_previous_x = 0;
 unsigned int display_confirm_previous_y = 0;
 unsigned int display_confirm_previous_w = 0;
 unsigned int display_confirm_previous_h = 0;
 unsigned int display_confirm_previous_desktop_w = 0;
 unsigned int display_confirm_previous_desktop_h = 0;
 ps2vnc_backend_t display_confirm_previous_backend =
    PS2VNC_BACKEND_NONE;

/*
 * D17AL-F3:
 * screen_fit_* is part of the complete local display profile.
 *
 * A provisional mode switch must not allow candidate B's calibration state
 * to survive a Go Back/timeout reconstruction of confirmed profile A.
 */
 int display_confirm_previous_screen_fit_width = 0;
 int display_confirm_previous_screen_fit_height = 0;
 int display_confirm_previous_screen_fit_offset_x = 0;
 int display_confirm_previous_screen_fit_offset_y = 0;
 int display_confirm_previous_screen_fit_full_bypass = 0;

volatile u64 display_confirm_started_tick = 0;

/*
 * H4B2C:
 * Successful rollback is not the end of the global transaction.
 *
 * Once complete profile A has been reconstructed and the Pi has accepted
 * RESTORED, this acknowledgement owns local UI until the main thread sends
 * ACK successfully.  The controller never performs management HTTP.
 */
 volatile int display_restore_ack_visible = 0;
 volatile int display_restore_ack_request = 0;
 const ps2vnc_video_mode_t *display_restore_ack_mode = NULL;

 volatile int display_menu_parent_system = 0;

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

 const ps2vnc_hotkey_binding_t *
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
 int runtime_hotkey_has_hold_extension(
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

volatile int system_menu_visible = 0;
volatile int system_menu_selection = SYSTEM_MENU_REFRESH;
volatile int system_menu_status = SYSTEM_MENU_STATUS_NONE;
volatile int system_menu_countdown_action = RUNTIME_ACTION_NONE;
volatile unsigned int system_menu_countdown_seconds = 0;
volatile int runtime_action_request = RUNTIME_ACTION_NONE;

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

volatile int system_refresh_state = SYSTEM_REFRESH_READY;
volatile u64 system_refresh_cooldown_started_tick = 0;

/* Local GS-format keyboard texture buffer. */
unsigned short osk_pixels[OSK_WIDTH * OSK_HEIGHT] __attribute__((aligned(128)));

 int screen_row = 0;

void screenf(const char *fmt, ...)
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

 void sanitize_text(char *s)
{
    int i;

    for (i = 0; s[i] != '\0'; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c < 32 || c > 126)
            s[i] = '.';
    }
}

 unsigned int read_be16(const unsigned char *p)
{
    return ((unsigned int)p[0] << 8) |
           ((unsigned int)p[1]);
}

 unsigned int read_be32(const unsigned char *p)
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
int region_fits_u32(
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


int send_exact(int sock, const void *data, int len)
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
 int send_rfb_message(int sock, const void *data, int len)
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
 int ps2vnc_discard_controller_rfb_queue(void)
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
int flush_rfb_outgoing(int sock)
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
void reset_rfb_transport_state(void)
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
 void debug_emit_idle_poll(void);
 void debug_emit_now(void);

 int ps2vnc_remote_control_poll_idle(void);
 int ps2vnc_remote_display_lease_expired(void);

/*
 * TEST12B carrier-loss detection:
 * recv_exact() is defined before the Ethernet helper itself.
 */
 int ethGetNetIFLinkStatus(void);

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

 volatile int rfb_message_boundary_local_yield = 0;
 volatile int rfb_confirm_live_yield_enabled = 0;
 volatile unsigned int rfb_confirm_wait_generation = 0;

 int recv_exact(int sock, void *data, int len)
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

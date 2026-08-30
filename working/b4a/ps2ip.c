/*
 * PS2VNC - test31 RX sentinel + raw rectangle-header trace
 *
 * Milestone 5:
 *   - PS2 Ethernet: 192.168.50.2/24
 *   - Connect to TigerVNC: 192.168.50.1:5900
 *   - Complete RFB 3.8 handshake using SecurityType None
 *   - Request RGB565 little-endian + Raw encoding
 *   - Receive the complete 640x480 framebuffer
 *   - Convert RGB565 -> PS2 GS A1B5G5R5
 *   - Upload as a gsKit texture
 *   - Continuously request complete 640x480 frames
 *   - Upload each frame as a gsKit texture
 *   - Display the Pi desktop live on the PS2
 *
 * Debug output deliberately avoids '\n' in scr_printf().
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

#include "ps2vnc_config_text.h"

#include <kernel.h>
#include <timer.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <debug.h>
#include <netman.h>
#include <ps2ip.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <sbv_patches.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <libpad.h>
#include <gsKit.h>
#include <gsHires.h>
#include <dmaKit.h>

/*
 * Physical video output and remote framebuffer capacity are independent.
 * Test15 introduces a mode profile so validated lower/higher modes can
 * eventually coexist in one ELF and be selected from ps2vnc.conf.
 */
/*
 * Test15B2: provision enough EE-side RGB565 storage for a genuine
 * high-resolution 1080i desktop.  Runtime desktop geometry remains
 * calibration-driven and may be smaller than this capacity.
 */
#define RFB_CAPACITY_WIDTH   1920
#define RFB_CAPACITY_HEIGHT  1080

typedef enum {
    PS2VNC_BACKEND_NONE     = 0,
    PS2VNC_BACKEND_STANDARD = 1u << 0,
    PS2VNC_BACKEND_HIRES    = 1u << 1
} ps2vnc_backend_t;

typedef enum {
    PS2VNC_DISPLAY_GROUP_TV_DTV = 0,
    PS2VNC_DISPLAY_GROUP_VGA_60,
    PS2VNC_DISPLAY_GROUP_VGA_OTHER,
    PS2VNC_DISPLAY_GROUP_COUNT
} ps2vnc_display_group_t;

typedef struct {
    const char *name;
    int gs_mode;
    int interlace;
    int field;

    /* Electrical/physical video raster. */
    unsigned int raster_width;
    unsigned int raster_height;

    /*
     * Ordinary gsKit drawing/framebuffer geometry.
     *
     * Interlaced FRAME modes may have a half-height GS drawing buffer while
     * still carrying a full-frame RFB desktop. Zero means Standard is not
     * available for this mode.
     */
    unsigned int standard_fb_width;
    unsigned int standard_fb_height;
    int standard_offset_x;
    int standard_offset_y;

    /*
     * Fixed RFB/logical desktop for Standard.
     *
     * Keep this separate from standard_fb_* so interlaced FRAME modes can
     * preserve the complete streamed frame while gsKit draws at half height.
     */
    unsigned int standard_logical_width;
    unsigned int standard_logical_height;

    /* gsKit HIRES drawing geometry and pass count. */
    unsigned int hires_width;
    unsigned int hires_height;
    unsigned int hires_passes;

    unsigned int allowed_backends;
    ps2vnc_backend_t recommended_backend;

    /* Presentation-only grouping used by the Display Modes page. */
    ps2vnc_display_group_t menu_group;
} ps2vnc_video_mode_t;

/*
 * H4B1:
 * A display profile is the indivisible runtime unit shared by the PS2
 * and Pi transaction layers.  During a provisional switch every one of
 * these fields may become active together, but none becomes confirmed
 * persistence until the transaction is committed.
 */
typedef struct {
    const ps2vnc_video_mode_t *mode;
    ps2vnc_backend_t backend;

    unsigned int logical_width;
    unsigned int logical_height;

    unsigned int output_x;
    unsigned int output_y;
    unsigned int output_width;
    unsigned int output_height;
} ps2vnc_display_profile_t;

/*
 * H4C3B2A:
 * Read-only PS2 representation of H4A's durable Pi transaction.
 */
typedef enum {
    PS2VNC_DISPLAY_TX_NONE = 0,
    PS2VNC_DISPLAY_TX_PROVISIONAL,
    PS2VNC_DISPLAY_TX_COMMITTING,
    PS2VNC_DISPLAY_TX_RESTORING,
    PS2VNC_DISPLAY_TX_RESTORED
} ps2vnc_display_tx_state_t;

typedef struct {
    ps2vnc_display_tx_state_t state;
    ps2vnc_display_profile_t previous;
    ps2vnc_display_profile_t candidate;
} ps2vnc_display_transaction_t;

/*
 * H4C startup preflight result.
 *
 * Later reconciliation stages consume this exact durable state rather than
 * re-querying authority after RFB startup has begun.
 */
static ps2vnc_display_transaction_t
    startup_display_transaction;

static int startup_display_transaction_loaded = 0;

/*
 * H4C3C1:
 * The confirmed profile is derived from the SAME configuration body that
 * startup applied. Do not re-fetch configuration later to reconstruct it.
 */
static ps2vnc_display_profile_t
    startup_confirmed_display_profile;

static int startup_confirmed_display_profile_loaded = 0;

static ps2vnc_display_profile_t
    startup_authoritative_display_profile;

static int startup_authoritative_display_profile_loaded = 0;

/*
 * Durable-state obligations retained for the later reconciliation stage.
 *
 * provisional: RESTORE -> repair A -> RESTORED -> ACK
 * restoring:              repair A -> RESTORED -> ACK
 * restored:                          verify A -> ACK
 */
static int startup_display_commit_finish_required = 0;
static int startup_display_restore_required = 0;
static int startup_display_restored_mark_required = 0;
static int startup_display_restore_ack_required = 0;

/*
 * D17AL large display-mode matrix.
 *
 * Raster geometry, renderer drawing geometry, and RFB logical geometry are
 * deliberately independent concepts.
 *
 * TV/DTV
 * -------
 * 480i Standard reconstructs the hardware-proven Test13C contract:
 *     physical timing : NTSC 480i
 *     GS drawing      : 704x232 FRAME
 *     RFB desktop     : 704x464
 *
 * 576i begins as HIRES so its full-frame logical geometry can be exercised
 * without inventing an unproven Standard PAL safe-area contract.
 *
 * VGA
 * ---
 * The initial VGA matrix is HIRES-only. It intentionally covers both widths
 * divisible by the GS 64-pixel FBW unit and widths that are not, giving the
 * mode survey useful presentation-path controls.
 */

static const ps2vnc_video_mode_t video_mode_480i = {
    "480i", GS_MODE_NTSC, GS_INTERLACED, GS_FRAME,
    720, 480,
    704, 232, -32, 8,
    704, 464,
    0, 0, 0,
    PS2VNC_BACKEND_STANDARD,
    PS2VNC_BACKEND_STANDARD,
    PS2VNC_DISPLAY_GROUP_TV_DTV
};

static const ps2vnc_video_mode_t video_mode_480p = {
    "480p", GS_MODE_DTV_480P, GS_NONINTERLACED, GS_FRAME,
    720, 480,
    704, 462, -4, 3,
    704, 462,
    0, 0, 0,
    PS2VNC_BACKEND_STANDARD,
    PS2VNC_BACKEND_STANDARD,
    PS2VNC_DISPLAY_GROUP_TV_DTV
};

static const ps2vnc_video_mode_t video_mode_480p_hires = {
    "480p-hires", GS_MODE_DTV_480P, GS_NONINTERLACED, GS_FRAME,
    720, 480,
    0, 0, 0, 0,
    0, 0,

    /*
     * D17AL-F7A:
     *
     * Keep the DTV 480p physical timing at 720x480, but give gsHires
     * the same native 704x462 drawing geometry already proven by the
     * ordinary-gsKit 480p backend.
     *
     * F7A is a local-primitive geometry probe only. RFB/background
     * publication is deliberately disabled for this one exact mode below.
     */
    704, 462, 3,

    PS2VNC_BACKEND_HIRES,
    PS2VNC_BACKEND_HIRES,
    PS2VNC_DISPLAY_GROUP_TV_DTV
};

static const ps2vnc_video_mode_t video_mode_576i = {
    "576i", GS_MODE_PAL, GS_INTERLACED, GS_FRAME,
    720, 576,
    0, 0, 0, 0,
    0, 0,
    720, 288, 3,
    PS2VNC_BACKEND_HIRES,
    PS2VNC_BACKEND_HIRES,
    PS2VNC_DISPLAY_GROUP_TV_DTV
};

static const ps2vnc_video_mode_t video_mode_576p = {
    "576p", GS_MODE_DTV_576P, GS_NONINTERLACED, GS_FRAME,
    720, 576,
    0, 0, 0, 0,
    0, 0,
    720, 576, 3,
    PS2VNC_BACKEND_HIRES,
    PS2VNC_BACKEND_HIRES,
    PS2VNC_DISPLAY_GROUP_TV_DTV
};

static const ps2vnc_video_mode_t video_mode_720p = {
    "720p", GS_MODE_DTV_720P, GS_NONINTERLACED, GS_FRAME,
    1280, 720,
    0, 0, 0, 0,
    0, 0,
    1280, 720, 3,
    PS2VNC_BACKEND_HIRES,
    PS2VNC_BACKEND_HIRES,
    PS2VNC_DISPLAY_GROUP_TV_DTV
};

static const ps2vnc_video_mode_t video_mode_1080i = {
    "1080i", GS_MODE_DTV_1080I, GS_INTERLACED, GS_FRAME,
    1920, 1080,
    0, 0, 0, 0,
    0, 0,
    1920, 540, 3,
    PS2VNC_BACKEND_HIRES,
    PS2VNC_BACKEND_HIRES,
    PS2VNC_DISPLAY_GROUP_TV_DTV
};


/* VGA 60 Hz family. */

static const ps2vnc_video_mode_t video_mode_vga_640_60 = {
    "640x480-60", GS_MODE_VGA_640_60, GS_NONINTERLACED, GS_FRAME,
    640, 480,
    0, 0, 0, 0,
    0, 0,
    640, 480, 3,
    PS2VNC_BACKEND_HIRES, PS2VNC_BACKEND_HIRES,
    PS2VNC_DISPLAY_GROUP_VGA_60
};

static const ps2vnc_video_mode_t video_mode_vga_800_60 = {
    "800x600-60", GS_MODE_VGA_800_60, GS_NONINTERLACED, GS_FRAME,
    800, 600,
    0, 0, 0, 0,
    0, 0,
    800, 600, 3,
    PS2VNC_BACKEND_HIRES, PS2VNC_BACKEND_HIRES,
    PS2VNC_DISPLAY_GROUP_VGA_60
};

static const ps2vnc_video_mode_t video_mode_vga_1024_60 = {
    "1024x768-60", GS_MODE_VGA_1024_60, GS_NONINTERLACED, GS_FRAME,
    1024, 768,
    0, 0, 0, 0,
    0, 0,
    1024, 768, 3,
    PS2VNC_BACKEND_HIRES, PS2VNC_BACKEND_HIRES,
    PS2VNC_DISPLAY_GROUP_VGA_60
};

static const ps2vnc_video_mode_t video_mode_vga_1280_60 = {
    "1280x1024-60", GS_MODE_VGA_1280_60, GS_NONINTERLACED, GS_FRAME,
    1280, 1024,
    0, 0, 0, 0,
    0, 0,
    1280, 1024, 3,
    PS2VNC_BACKEND_HIRES, PS2VNC_BACKEND_HIRES,
    PS2VNC_DISPLAY_GROUP_VGA_60
};


/* VGA alternate-refresh family. */

#define D17AL_VGA_OTHER(var, label, gsconst, w, h) \
static const ps2vnc_video_mode_t var = { \
    label, gsconst, GS_NONINTERLACED, GS_FRAME, \
    w, h, \
    0, 0, 0, 0, \
    0, 0, \
    w, h, 3, \
    PS2VNC_BACKEND_HIRES, PS2VNC_BACKEND_HIRES, \
    PS2VNC_DISPLAY_GROUP_VGA_OTHER \
}

D17AL_VGA_OTHER(
    video_mode_vga_640_72,
    "640x480-72", GS_MODE_VGA_640_72, 640, 480);

D17AL_VGA_OTHER(
    video_mode_vga_640_75,
    "640x480-75", GS_MODE_VGA_640_75, 640, 480);

D17AL_VGA_OTHER(
    video_mode_vga_640_85,
    "640x480-85", GS_MODE_VGA_640_85, 640, 480);

D17AL_VGA_OTHER(
    video_mode_vga_800_56,
    "800x600-56", GS_MODE_VGA_800_56, 800, 600);

D17AL_VGA_OTHER(
    video_mode_vga_800_72,
    "800x600-72", GS_MODE_VGA_800_72, 800, 600);

D17AL_VGA_OTHER(
    video_mode_vga_800_75,
    "800x600-75", GS_MODE_VGA_800_75, 800, 600);

D17AL_VGA_OTHER(
    video_mode_vga_800_85,
    "800x600-85", GS_MODE_VGA_800_85, 800, 600);

D17AL_VGA_OTHER(
    video_mode_vga_1024_70,
    "1024x768-70", GS_MODE_VGA_1024_70, 1024, 768);

D17AL_VGA_OTHER(
    video_mode_vga_1024_75,
    "1024x768-75", GS_MODE_VGA_1024_75, 1024, 768);

D17AL_VGA_OTHER(
    video_mode_vga_1024_85,
    "1024x768-85", GS_MODE_VGA_1024_85, 1024, 768);

D17AL_VGA_OTHER(
    video_mode_vga_1280_75,
    "1280x1024-75", GS_MODE_VGA_1280_75, 1280, 1024);

#undef D17AL_VGA_OTHER


/*
 * Registry order is also the stable transaction/request identity.
 * Keep entries grouped and sorted for human inspection; the UI derives its
 * columns from menu_group rather than assuming a particular row count.
 */
static const ps2vnc_video_mode_t *const video_modes[] = {
    /* TV / DTV */
    &video_mode_480i,
    &video_mode_480p,
    &video_mode_480p_hires,
    &video_mode_576i,
    &video_mode_576p,
    &video_mode_720p,
    &video_mode_1080i,

    /* VGA 60 Hz */
    &video_mode_vga_640_60,
    &video_mode_vga_800_60,
    &video_mode_vga_1024_60,
    &video_mode_vga_1280_60,

    /* VGA alternate refresh */
    &video_mode_vga_640_72,
    &video_mode_vga_640_75,
    &video_mode_vga_640_85,
    &video_mode_vga_800_56,
    &video_mode_vga_800_72,
    &video_mode_vga_800_75,
    &video_mode_vga_800_85,
    &video_mode_vga_1024_70,
    &video_mode_vga_1024_75,
    &video_mode_vga_1024_85,
    &video_mode_vga_1280_75
};

#define VIDEO_MODE_COUNT \
    (sizeof(video_modes) / sizeof(video_modes[0]))

static int ps2vnc_video_mode_supports_backend(
    const ps2vnc_video_mode_t *mode,
    ps2vnc_backend_t backend)
{
    if (mode == NULL || backend == PS2VNC_BACKEND_NONE)
        return 0;

    return (mode->allowed_backends & (unsigned int)backend) != 0;
}

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


static int ps2vnc_config_parse_int(
    const char *text,
    int allow_negative,
    int require_positive,
    int *out_value)
{
    const char *p;
    const char *digits;

    char *endptr;

    unsigned long magnitude = 0;
    unsigned long limit;

    int negative = 0;

    long value;

    if (text == NULL ||
        out_value == NULL ||
        text[0] == '\0')
        return 0;

    p = text;

    if (*p == '-') {

        if (!allow_negative)
            return 0;

        negative = 1;
        p++;

        if (*p == '\0')
            return 0;

    } else if (*p == '+') {

        /*
         * Keep one normalized representation.
         * A leading plus sign is unnecessary and rejected.
         */
        return 0;
    }

    digits = p;

    /*
     * Preflight the decimal magnitude ourselves.
     *
     * The PS2 libc strtol() observed in D4-A2 can saturate on overflow
     * without reliably reporting ERANGE.  Therefore errno alone is not a
     * sufficient range boundary on this target.
     *
     * Accumulate only while the next decimal digit provably fits inside
     * the destination int range.  This avoids arithmetic overflow in the
     * checker itself.
     */
    if (negative) {
        limit = (unsigned long)INT_MAX + 1UL;
    } else {
        limit = (unsigned long)INT_MAX;
    }

    while (*p != '\0') {
        unsigned int digit;

        if (*p < '0' || *p > '9')
            return 0;

        digit = (unsigned int)(*p - '0');

        if (magnitude >
                (limit - (unsigned long)digit) / 10UL) {

            return 0;
        }

        magnitude =
            magnitude * 10UL +
            (unsigned long)digit;

        p++;
    }

    if (p == digits)
        return 0;

    /*
     * Keep strtol() as the final conversion and complete-string check,
     * but do not depend on libc to be our only overflow detector.
     */
    errno = 0;
    endptr = NULL;

    value = strtol(
        text,
        &endptr,
        10
    );

    if (errno == ERANGE)
        return 0;

    if (endptr == NULL ||
        endptr == text ||
        *endptr != '\0')
        return 0;

    if (value < INT_MIN ||
        value > INT_MAX)
        return 0;

    if (require_positive &&
        value <= 0)
        return 0;

    *out_value = (int)value;

    return 1;
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


static int ps2vnc_config_parse_bool(
    const char *value,
    int *out_value)
{
    if (value == NULL ||
        out_value == NULL)
        return 0;

    if (strcmp(value, "true") == 0 ||
        strcmp(value, "1") == 0) {

        *out_value = 1;
        return 1;
    }

    if (strcmp(value, "false") == 0 ||
        strcmp(value, "0") == 0) {

        *out_value = 0;
        return 1;
    }

    return 0;
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
static int ps2vnc_switch_display_mode_core(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    GSGLOBAL **display,
    GSTEXTURE *tex,
    GSTEXTURE *osk_tex,
    const ps2vnc_video_mode_t *mode,
    unsigned int out_x,
    unsigned int out_y,
    unsigned int width,
    unsigned int height)
{
    unsigned int logical_width;

    display_switch_failure_stage = 0;
    live_error_code = 0;
    unsigned int logical_height;

    if (mode == NULL || display == NULL)
{
            display_switch_failure_stage = 1u;
            return -1;
        }

    if (!region_fits_u32(
            out_x, out_y, width, height,
            mode->raster_width, mode->raster_height))
{
            display_switch_failure_stage = 2u;
            return -1;
        }

    /*
     * Main thread remains the sole RFB socket writer. Empty any controller
     * traffic accumulated before entering the display transaction.
     */
    if (rfb_queue_sema >= 0 &&
        flush_rfb_outgoing(sock) < 0)
{
            display_switch_failure_stage = 3u;
            return -1;
        }

    ps2vnc_display_direct_write_cancel();

    if (ps2vnc_display_destroy(display) < 0)
{
            display_switch_failure_stage = 4u;
            return -1;
        }

    active_video_mode = mode;

    *display = ps2vnc_display_create();
    if (*display == NULL)
{
            display_switch_failure_stage = 5u;
            return -1;
        }

    /*
     * Re-advertise EDS because ordinary live mode advertises only
     * Hextile + Raw.
     */
    if (set_raw_extended_desktop_encoding(sock) < 0)
{
            display_switch_failure_stage = 6u;
            return -1;
        }

    if (ps2vnc_display_logical_desktop_size(
            width, height,
            &logical_width, &logical_height) < 0)
{
            display_switch_failure_stage = 7u;
            return -1;
        }

    if (desktop_width != logical_width ||
        desktop_height != logical_height) {
        if (request_and_confirm_desktop_size(
                sock,
                rfb_framebuffer,
                gs_framebuffer,
                logical_width,
                logical_height) < 0)
{
                display_switch_failure_stage = 8u;
                return -1;
            }
    }

    desktop_width = logical_width;
    desktop_height = logical_height;
    desktop_output_x = out_x;
    desktop_output_y = out_y;
    desktop_output_width = width;
    desktop_output_height = height;
    desktop_geometry_locked = 1;

    debug_emit_geometry_snapshot();

    /*
     * Build an authoritative framebuffer at the backend-selected logical
     * desktop geometry before preparing the reconstructed presentation.
     */
    if (set_raw_encoding(sock) < 0)
{
            display_switch_failure_stage = 9u;
            return -1;
        }

    if (request_framebuffer(sock, 0) < 0)
{
            display_switch_failure_stage = 10u;
            return -1;
        }

    if (receive_framebuffer_update(
            sock,
            rfb_framebuffer,
            gs_framebuffer,
            0,
            1) < 0)
{
            display_switch_failure_stage = 11u;
            return -1;
        }

    if (ps2vnc_display_prepare_desktop(
            *display,
            tex,
            gs_framebuffer) < 0)
{
            display_switch_failure_stage = 12u;
            return -1;
        }

    ps2vnc_display_prepare_osk(*display, osk_tex);

    if (set_live_encoding(sock) < 0)
{
            display_switch_failure_stage = 13u;
            return -1;
        }

    /*
     * The OSK texture now belongs to a new GS VRAM instance.
     */
    osk_generation++;

    return 0;
}


/*
 * D17AL-F8J2-B4A — hard remote transition curtain.
 *
 * Every one-way remote test transition follows:
 *
 * controller OFF
 *   -> discard queued controller RFB
 *   -> curtain ON
 *   -> 500 ms source settle
 *   -> existing B3C display reconstruction
 *   -> curtain ON in destination timing
 *   -> 500 ms destination settle
 *   -> complete destination desktop presentation
 *   -> curtain OFF
 *   -> controller ON
 *
 * Forward and rollback both enter through this wrapper. The controller
 * remains fully active during the ordinary temporary-mode dwell.
 *
 * The controller thread never touches the VNC socket here. The main
 * thread remains the sole owner of RFB socket traffic.
 */
static int ps2vnc_switch_display_mode(
    int sock,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    GSGLOBAL **display,
    GSTEXTURE *tex,
    GSTEXTURE *osk_tex,
    const ps2vnc_video_mode_t *mode,
    unsigned int out_x,
    unsigned int out_y,
    unsigned int width,
    unsigned int height)
{
    const struct timespec settle_delay = {
        0,
        500000000
    };

    const u64 black =
        GS_SETREG_RGBAQ(
            0x00, 0x00, 0x00,
            0x80, 0x00);

    const u64 tex_color =
        GS_SETREG_RGBAQ(
            0x80, 0x80, 0x80,
            0x80, 0x00);

    int controller_acquired = 0;
    int result = -1;

    /*
     * B4A is deliberately restricted to the backdoor remote display
     * transaction path. Preserve ordinary local UI behavior exactly.
     */
    if (remote_display_state == REMOTE_DISPLAY_IDLE) {
        return ps2vnc_switch_display_mode_core(
            sock,
            rfb_framebuffer,
            gs_framebuffer,
            display,
            tex,
            osk_tex,
            mode,
            out_x,
            out_y,
            width,
            height);
    }

    display_switch_failure_stage = 0;
    live_error_code = 0;

    if (display == NULL ||
        *display == NULL ||
        tex == NULL ||
        osk_tex == NULL ||
        mode == NULL) {
        display_switch_failure_stage = 14u;
        return -1;
    }

    /*
     * True controller disconnect: the controller thread acknowledges
     * immediately before its next libpad access and performs no further
     * padGetState()/padRead() while ownership is held here.
     */
    if (ps2vnc_controller_pad_acquire() < 0) {
        display_switch_failure_stage = 14u;
        return -1;
    }

    controller_acquired = 1;

    /*
     * Drop any pointer/key/scroll messages generated before the pause ACK.
     * Do not send them into the display reconstruction epoch.
     */
    if (ps2vnc_discard_controller_rfb_queue() < 0) {
        display_switch_failure_stage = 15u;
        goto out;
    }

    debug_transition_phase = 1u;
    debug_emit_now();

    /*
     * Source-timing curtain.
     */
    if (draw_recovery_status(
            *display,
            osk_tex,
            "PLEASE WAIT",
            "CHANGING DISPLAY MODES",
            0) < 0) {
        display_switch_failure_stage = 16u;
        goto out;
    }

    debug_transition_phase = 2u;
    debug_emit_now();

    nanosleep(&settle_delay, NULL);

    /*
     * Exact B3C reconstruction body.
     */
    debug_transition_phase = 3u;
    debug_emit_now();

    result = ps2vnc_switch_display_mode_core(
        sock,
        rfb_framebuffer,
        gs_framebuffer,
        display,
        tex,
        osk_tex,
        mode,
        out_x,
        out_y,
        width,
        height);

    if (result < 0)
        goto out;

    debug_transition_phase = 4u;
    debug_emit_now();

    /*
     * The destination GS/RFB reconstruction is now complete. Repaint
     * the same curtain in the destination timing before exposing the
     * reconstructed desktop.
     */
    if (draw_recovery_status(
            *display,
            osk_tex,
            "PLEASE WAIT",
            "CHANGING DISPLAY MODES",
            0) < 0) {
        display_switch_failure_stage = 17u;
        result = -1;
        goto out;
    }

    debug_transition_phase = 5u;
    debug_emit_now();

    nanosleep(&settle_delay, NULL);

    /*
     * Curtain up: explicitly present one coherent destination desktop
     * frame before controller ownership returns.
     */
    if (ps2vnc_display_draw_desktop(
            *display,
            tex,
            black,
            tex_color) < 0) {
        display_switch_failure_stage = 18u;
        result = -1;
        goto out;
    }

    if (ps2vnc_display_finish_draw(*display) < 0) {
        display_switch_failure_stage = 19u;
        result = -1;
        goto out;
    }

    if (ps2vnc_display_flip(*display) < 0) {
        display_switch_failure_stage = 20u;
        result = -1;
        goto out;
    }

    debug_transition_phase = 6u;
    debug_emit_now();

    result = 0;

out:
    /*
     * Do NOT call ps2vnc_controller_pad_wait_release().
     * The hostile-stick benchmark deliberately holds the analog stick.
     * The controller thread's existing resume path clears pre-handoff
     * edge/hold/analog/scroll state before normal polling resumes.
     */
    if (controller_acquired) {
        if (ps2vnc_controller_pad_release() < 0) {
            if (result == 0)
                display_switch_failure_stage = 21u;

            result = -1;
            debug_emit_now();
            return result;
        }

        debug_transition_phase = 7u;
        debug_emit_now();
    }

    debug_transition_phase = 0u;
    debug_emit_now();

    return result;
}


/*
 * H3C2A complete known-good display rollback.
 *
 * The suspect RFB stream is abandoned first. Only after a fresh synchronized
 * session exists do we reconstruct the saved display configuration.
 */
static int ps2vnc_rollback_display_transaction(
    int old_sock,
    int transport_dirty,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    GSGLOBAL **display,
    GSTEXTURE *tex,
    GSTEXTURE *osk_tex)
{
    const ps2vnc_video_mode_t *previous_mode =
        display_confirm_previous_mode;

    const ps2vnc_backend_t previous_backend =
        display_confirm_previous_backend;

    const unsigned int previous_x =
        display_confirm_previous_x;
    const unsigned int previous_y =
        display_confirm_previous_y;
    const unsigned int previous_w =
        display_confirm_previous_w;
    const unsigned int previous_h =
        display_confirm_previous_h;

    const unsigned int previous_desktop_w =
        display_confirm_previous_desktop_w;
    const unsigned int previous_desktop_h =
        display_confirm_previous_desktop_h;

    const int previous_screen_fit_width =
        display_confirm_previous_screen_fit_width;
    const int previous_screen_fit_height =
        display_confirm_previous_screen_fit_height;
    const int previous_screen_fit_offset_x =
        display_confirm_previous_screen_fit_offset_x;
    const int previous_screen_fit_offset_y =
        display_confirm_previous_screen_fit_offset_y;
    const int previous_screen_fit_full_bypass =
        display_confirm_previous_screen_fit_full_bypass;

    int new_sock;

    if (!display_confirm_pending ||
        previous_mode == NULL ||
        previous_backend == PS2VNC_BACKEND_NONE ||
        previous_desktop_w == 0 ||
        previous_desktop_h == 0 ||
        previous_screen_fit_width <= 0 ||
        previous_screen_fit_height <= 0)
        return -1;

    /*
     * Claim rollback before reconnecting. recv_exact() interrupts only the
     * unclaimed ROLLBACK state, never ROLLBACK_IN_PROGRESS.
     */
    display_confirm_action =
        DISPLAY_CONFIRM_ACTION_ROLLBACK_IN_PROGRESS;
    display_confirm_visible = 0;
    osk_generation++;

    /*
     * A completed candidate leaves us at a synchronized RFB message
     * boundary. Go Back / confirmation timeout therefore reuses that
     * healthy session and resizes it directly back to known-good.
     *
     * Only an interrupted candidate or genuine RFB failure owns a suspect
     * byte stream and requires transport replacement first.
     */
    if (transport_dirty) {
        new_sock =
            recover_rfb_transport_for_display_rollback(
                old_sock,
                rfb_framebuffer,
                gs_framebuffer,
                previous_mode);

        if (new_sock < 0)
            return -1;
    } else {
        if (old_sock < 0)
            return -1;

        new_sock = old_sock;
    }

    if (ps2vnc_switch_display_mode(
            new_sock,
            rfb_framebuffer,
            gs_framebuffer,
            display,
            tex,
            osk_tex,
            previous_mode,
            previous_x,
            previous_y,
            previous_w,
            previous_h) < 0) {

        close(new_sock);
        reset_rfb_transport_state();
        return -1;
    }

    /*
     * D17AL-F3:
     * ps2vnc_switch_display_mode() reconstructs A's GS/RFB presentation,
     * but screen-fit state is an independent component of the local profile.
     * Restore it only after A is active again.
     */
    if (!screen_fit_apply_preset(
            previous_screen_fit_width,
            previous_screen_fit_height,
            previous_screen_fit_offset_x,
            previous_screen_fit_offset_y,
            previous_screen_fit_full_bypass)) {

        close(new_sock);
        reset_rfb_transport_state();
        return -1;
    }

    /*
     * Verify that the exact saved known-good configuration came back.
     */
    if (active_video_mode != previous_mode ||
        active_display_backend != previous_backend ||
        desktop_width != previous_desktop_w ||
        desktop_height != previous_desktop_h ||
        desktop_output_x != previous_x ||
        desktop_output_y != previous_y ||
        desktop_output_width != previous_w ||
        desktop_output_height != previous_h ||
        screen_fit_width != previous_screen_fit_width ||
        screen_fit_height != previous_screen_fit_height ||
        screen_fit_offset_x != previous_screen_fit_offset_x ||
        screen_fit_offset_y != previous_screen_fit_offset_y ||
        screen_fit_full_bypass !=
            previous_screen_fit_full_bypass) {

        close(new_sock);
        reset_rfb_transport_state();
        return -1;
    }

    rfb_connected = 1;
    display_confirm_disarm_transaction();

    return new_sock;
}

/*
 * H4B2B:
 * One global rollback boundary for every rollback cause.
 *
 * The durable Pi transaction enters RESTORING before the existing
 * complete local/RFB reconstruction of confirmed profile A. Only after
 * A has been reconstructed successfully may the Pi record RESTORED.
 *
 * If either management transition fails, the best-effort local restore
 * still runs, but the caller receives failure: global reconciliation is
 * incomplete and must never be silently treated as success.
 */
/*
 * Publish the existing H4B2C restored acknowledgement UI.
 *
 * The durable transaction remains RESTORED until controller X causes the
 * main thread to POST ACK successfully.
 */
static void ps2vnc_publish_restored_ack(
    const ps2vnc_video_mode_t *mode)
{
    display_restore_ack_mode =
        (mode != NULL) ? mode : active_video_mode;

    display_restore_ack_request = 0;

    system_menu_visible = 0;
    display_menu_visible = 0;
    osk_visible = 0;

    display_restore_ack_visible = 1;
    osk_generation++;

    screenf(
        "DISPLAY TX: restored %s; awaiting ACK",
        display_restore_ack_mode ?
            display_restore_ack_mode->name : "?");
}


/*
 * Complete durable startup recovery ONLY after the entire local runtime
 * profile exists:
 *
 *   - authoritative mode selected
 *   - authoritative backend created
 *   - TigerVNC at authoritative logical geometry
 *   - output/presentation rectangle installed
 *   - complete Raw framebuffer obtained
 *   - desktop + OSK presentation prepared
 *
 * Capture the live profile and compare it byte-for-field semantically with
 * the selected authority before changing durable transaction state.
 */
static int ps2vnc_startup_finalize_display_transaction(void)
{
    ps2vnc_display_profile_t live_profile;
    const ps2vnc_display_profile_t *target;

    if (!startup_display_transaction_loaded)
        return -1;

    if (!startup_authoritative_display_profile_loaded) {

        return startup_display_transaction.state ==
            PS2VNC_DISPLAY_TX_NONE ? 0 : -1;
    }

    target = &startup_authoritative_display_profile;

    if (ps2vnc_display_profile_capture_active(
            &live_profile) < 0 ||
        !ps2vnc_display_tx_profiles_equal(
            &live_profile,
            target)) {

        screenf("DISPLAY TX: runtime profile mismatch");
        return -1;
    }

    switch (startup_display_transaction.state) {

        case PS2VNC_DISPLAY_TX_NONE:

            if (startup_display_commit_finish_required ||
                startup_display_restored_mark_required ||
                startup_display_restore_ack_required) {

                screenf("DISPLAY TX: none obligations invalid");
                return -1;
            }

            return 0;


        case PS2VNC_DISPLAY_TX_COMMITTING:

            if (!startup_display_commit_finish_required) {
                screenf("DISPLAY TX: commit obligation missing");
                return -1;
            }

            /*
             * B is already complete and verified.  The idempotent Pi COMMIT
             * now promotes/persists that already-running B; it does not
             * reconstruct B.
             */
            if (ps2vnc_display_transaction_commit_to_pi() < 0) {
                screenf("DISPLAY TX: startup COMMIT failed");
                return -1;
            }

            startup_display_commit_finish_required = 0;
            startup_display_transaction.state =
                PS2VNC_DISPLAY_TX_NONE;

            screenf(
                "DISPLAY TX: startup committed %s",
                target->mode->name);

            return 0;


        case PS2VNC_DISPLAY_TX_PROVISIONAL:

            /*
             * prepare_display_transaction() must have durably converted this
             * to RESTORING before any reconstruction started.
             */
            screenf("DISPLAY TX: provisional escaped preparation");
            return -1;


        case PS2VNC_DISPLAY_TX_RESTORING:

            if (!startup_display_restored_mark_required ||
                !startup_display_restore_ack_required) {

                screenf("DISPLAY TX: restore obligations invalid");
                return -1;
            }

            /*
             * Complete A is now reconstructed and exactly verified.
             * Only now may the Pi enter durable RESTORED.
             */
            if (ps2vnc_display_transaction_restored_to_pi() < 0) {
                screenf("DISPLAY TX: startup RESTORED failed");
                return -1;
            }

            startup_display_restored_mark_required = 0;
            startup_display_transaction.state =
                PS2VNC_DISPLAY_TX_RESTORED;

            ps2vnc_publish_restored_ack(target->mode);

            return 0;


        case PS2VNC_DISPLAY_TX_RESTORED:

            if (!startup_display_restore_ack_required) {
                screenf("DISPLAY TX: restored ACK obligation missing");
                return -1;
            }

            /*
             * A was already durably RESTORED before this process started.
             * Reassert/verify complete A, but deliberately leave the Pi in
             * RESTORED until the user presses X on the ACK UI.
             */
            ps2vnc_publish_restored_ack(target->mode);

            return 0;


        default:
            return -1;
    }
}



static int ps2vnc_remote_control_get_value(
    const char *text,
    const char *key,
    char *out,
    unsigned int out_size)
{
    const char *line;
    unsigned int key_len;

    if (text == NULL ||
        key == NULL ||
        out == NULL ||
        out_size == 0)
        return 0;

    key_len = (unsigned int)strlen(key);
    line = text;

    while (*line != '\0') {
        const char *end = strchr(line, '\n');
        const char *value;
        unsigned int len;

        if (end == NULL)
            end = line + strlen(line);

        if ((unsigned int)(end - line) > key_len &&
            strncmp(line, key, key_len) == 0 &&
            line[key_len] == '=') {

            value = line + key_len + 1;
            len = (unsigned int)(end - value);

            if (len > 0 &&
                value[len - 1] == '\r')
                len--;

            if (len >= out_size)
                return 0;

            memcpy(out, value, len);
            out[len] = '\0';
            return 1;
        }

        if (*end == '\0')
            break;

        line = end + 1;
    }

    return 0;
}


static int ps2vnc_remote_control_get_u32(
    const char *text,
    const char *key,
    unsigned int *out)
{
    char value[32];
    char *end = NULL;
    unsigned long parsed;

    if (out == NULL ||
        !ps2vnc_remote_control_get_value(
            text,
            key,
            value,
            sizeof(value)))
        return 0;

    if (value[0] == '\0')
        return 0;

    parsed = strtoul(value, &end, 10);

    if (end == value ||
        *end != '\0')
        return 0;

    *out = (unsigned int)parsed;
    return 1;
}


static int ps2vnc_remote_result_post(
    const char *state,
    unsigned int error_code)
{
    char body[512];
    int n;

    const char *active_name =
        (active_video_mode != NULL &&
         active_video_mode->name != NULL)
            ? active_video_mode->name
            : "";

    const char *baseline_name =
        (remote_display_baseline_mode != NULL &&
         remote_display_baseline_mode->name != NULL)
            ? remote_display_baseline_mode->name
            : active_name;

    if (remote_display_txid == 0 ||
        state == NULL)
        return -1;

    n = snprintf(
        body,
        sizeof(body),
        "txid=%u\n"
        "state=%s\n"
        "active_mode=%s\n"
        "baseline_mode=%s\n"
        "error_code=%u\n",
        remote_display_txid,
        state,
        active_name,
        baseline_name,
        error_code);

    if (n <= 0 ||
        n >= (int)sizeof(body))
        return -1;

    return ps2vnc_management_post_text(
        PS2VNC_DISPLAY_CONTROL_RESULT_PATH,
        body);
}


static void ps2vnc_remote_display_reset(void)
{
    remote_display_state = REMOTE_DISPLAY_IDLE;
    display_mode_request_remote = 0;

    remote_display_txid = 0;
    remote_display_lease_seconds = 0;
    remote_display_error_code = 0;

    remote_display_lease_started_tick = 0;

    remote_display_requested_mode = NULL;
    remote_display_baseline_mode = NULL;
}


static void ps2vnc_remote_display_fail(
    unsigned int error_code)
{
    remote_display_error_code = error_code;

    (void)ps2vnc_remote_result_post(
        "FAILED",
        error_code);

    ps2vnc_remote_display_reset();
}


static int ps2vnc_remote_display_lease_expired(void)
{
    u32 sec = 0;
    u32 usec = 0;

    if (remote_display_state !=
            REMOTE_DISPLAY_ACTIVE_TEMP ||
        !display_confirm_pending ||
        display_confirm_action !=
            DISPLAY_CONFIRM_ACTION_NONE)
        return 0;

    TimerBusClock2USec(
        GetTimerSystemTime() -
            remote_display_lease_started_tick,
        &sec,
        &usec);

    if (sec < remote_display_lease_seconds)
        return 0;

    screenf(
        "REMOTE DISPLAY: lease expired tx=%u",
        remote_display_txid);

    remote_display_state =
        REMOTE_DISPLAY_RESTORING;

    remote_display_error_code = 0;

    (void)ps2vnc_remote_result_post(
        "RESTORING",
        0);

    display_confirm_action =
        DISPLAY_CONFIRM_ACTION_ROLLBACK;

    return 1;
}


static int ps2vnc_remote_control_poll_idle(void)
{
    unsigned char body[
        PS2VNC_DISPLAY_CONTROL_MAX_BYTES + 1];

    unsigned int len = 0;
    unsigned int txid;
    unsigned int lease_seconds;

    char state[32];
    char action[32];
    char mode_name[64];

    const ps2vnc_video_mode_t *target;
    u64 now = GetTimerSystemTime();

    if (remote_display_last_poll_tick != 0 &&
        profile_ticks_us(
            now -
            remote_display_last_poll_tick) <
            PS2VNC_REMOTE_CONTROL_POLL_US)
        return 0;

    remote_display_last_poll_tick = now;

    if (ps2vnc_management_get_bounded_quiet(
            PS2VNC_DISPLAY_CONTROL_PENDING_PATH,
            body,
            PS2VNC_DISPLAY_CONTROL_MAX_BYTES,
            &len,
            PS2VNC_REMOTE_CONTROL_GET_BUDGET_US) < 0) {

        /*
         * Development control is best-effort.
         * A missing Pi management service must never damage VNC.
         */
        return 0;
    }

    if (len == 0 ||
        len > PS2VNC_DISPLAY_CONTROL_MAX_BYTES)
        return 0;

    body[len] = '\0';

    if (!ps2vnc_remote_control_get_value(
            (const char *)body,
            "state",
            state,
            sizeof(state)))
        return 0;

    if (strcmp(state, "none") == 0)
        return 0;

    if (!ps2vnc_remote_control_get_u32(
            (const char *)body,
            "txid",
            &txid) ||
        !ps2vnc_remote_control_get_value(
            (const char *)body,
            "action",
            action,
            sizeof(action)))
        return 0;

    if (strcmp(state, "PENDING") == 0 &&
        strcmp(action, "set-temp") == 0) {

        /*
         * Wait rather than steal ownership from a local transaction.
         */
        if (display_confirm_pending ||
            display_transition_switch_in_progress ||
            display_mode_request !=
                DISPLAY_MODE_REQUEST_NONE ||
            display_calibration_request !=
                DISPLAY_MODE_REQUEST_NONE ||
            controller_pad_pause_requested ||
            remote_display_state !=
                REMOTE_DISPLAY_IDLE)
            return 0;

        if (!ps2vnc_remote_control_get_value(
                (const char *)body,
                "mode",
                mode_name,
                sizeof(mode_name)) ||
            !ps2vnc_remote_control_get_u32(
                (const char *)body,
                "lease_seconds",
                &lease_seconds) ||
            lease_seconds == 0)
            return 0;

        target =
            ps2vnc_video_mode_by_name(mode_name);

        remote_display_txid = txid;
        remote_display_lease_seconds =
            lease_seconds;
        remote_display_error_code = 0;

        remote_display_requested_mode = target;
        remote_display_baseline_mode =
            active_video_mode;

        if (target == NULL) {
            ps2vnc_remote_display_fail(1);
            return 0;
        }

        if (target == active_video_mode) {
            ps2vnc_remote_display_fail(2);
            return 0;
        }

        remote_display_state =
            REMOTE_DISPLAY_SWITCH_REQUESTED;

        display_mode_request_remote = 1;
        display_mode_request =
            display_mode_request_for_selection(
                display_menu_selection_for_mode(
                    target));

        if (display_mode_request ==
                DISPLAY_MODE_REQUEST_NONE) {
            ps2vnc_remote_display_fail(3);
            return 0;
        }

        screenf(
            "REMOTE DISPLAY: tx=%u set-temp %s lease=%us",
            txid,
            target->name,
            lease_seconds);

        return 1;
    }

    if (strcmp(state, "RESTORE_PENDING") == 0 &&
        strcmp(action, "restore") == 0) {

        if (remote_display_state !=
                REMOTE_DISPLAY_ACTIVE_TEMP ||
            txid != remote_display_txid)
            return 0;

        remote_display_state =
            REMOTE_DISPLAY_RESTORING;

        remote_display_error_code = 0;

        (void)ps2vnc_remote_result_post(
            "RESTORING",
            0);

        display_confirm_action =
            DISPLAY_CONFIRM_ACTION_ROLLBACK;

        screenf(
            "REMOTE DISPLAY: explicit restore tx=%u",
            txid);

        return 1;
    }

    return 0;
}


static int ps2vnc_global_rollback_display_transaction(
    int old_sock,
    int transport_dirty,
    unsigned short *rfb_framebuffer,
    unsigned short *gs_framebuffer,
    GSGLOBAL **display,
    GSTEXTURE *tex,
    GSTEXTURE *osk_tex)
{
    int restore_marked;
    int new_sock;

    const int remote_restore =
        remote_display_state != REMOTE_DISPLAY_IDLE;

    if (remote_restore) {
        remote_display_suppress_next_active_menu_return = 1;

        remote_display_state =
            REMOTE_DISPLAY_RESTORING;

        (void)ps2vnc_remote_result_post(
            "RESTORING",
            remote_display_error_code);
    }

    restore_marked =
        ps2vnc_display_transaction_restore_to_pi() == 0;

    if (!restore_marked)
        screenf("DISPLAY TX: restore marker failed");

    new_sock = ps2vnc_rollback_display_transaction(
        old_sock,
        transport_dirty,
        rfb_framebuffer,
        gs_framebuffer,
        display,
        tex,
        osk_tex);

    if (new_sock < 0) {
        if (remote_restore)
            (void)ps2vnc_remote_result_post(
                "FAILED",
                remote_display_error_code != 0
                    ? remote_display_error_code
                    : 90);
        return -1;
    }

    if (!restore_marked) {
        screenf("DISPLAY TX: Pi reconciliation pending");

        if (remote_restore)
            (void)ps2vnc_remote_result_post(
                "FAILED",
                remote_display_error_code != 0
                    ? remote_display_error_code
                    : 91);

        return -1;
    }

    if (ps2vnc_display_transaction_restored_to_pi() < 0) {
        screenf("DISPLAY TX: restored marker failed");

        if (remote_restore)
            (void)ps2vnc_remote_result_post(
                "FAILED",
                remote_display_error_code != 0
                    ? remote_display_error_code
                    : 92);

        return -1;
    }

    if (remote_restore) {
        /*
         * Remote test rollback is machine-owned. The runtime is already
         * back at complete A and Pi is durably RESTORED, so finish ACK
         * without presenting the human restored-acknowledgement modal.
         */
        if (ps2vnc_display_transaction_ack_to_pi() < 0) {
            screenf(
                "REMOTE DISPLAY: automatic ACK failed");

            (void)ps2vnc_remote_result_post(
                "FAILED",
                remote_display_error_code != 0
                    ? remote_display_error_code
                    : 93);

            /*
             * Do not hide unresolved durability from the user.
             */
            ps2vnc_publish_restored_ack(
                active_video_mode);

            ps2vnc_remote_display_reset();
            return new_sock;
        }

        if (remote_display_error_code != 0) {
            (void)ps2vnc_remote_result_post(
                "FAILED",
                remote_display_error_code);
        } else {
            (void)ps2vnc_remote_result_post(
                "RESTORED",
                0);
        }

        screenf(
            "REMOTE DISPLAY: restored %s",
            active_video_mode ?
                active_video_mode->name : "?");

        ps2vnc_remote_display_reset();

        return new_sock;
    }

    ps2vnc_publish_restored_ack(
        (display_confirm_previous_mode != NULL)
            ? display_confirm_previous_mode
            : active_video_mode);

    return new_sock;
}



static int display_live_desktop(int sock,
                                unsigned short *rfb_framebuffer,
                                unsigned short *gs_framebuffer)
{
    GSGLOBAL *gsGlobal;
    GSTEXTURE tex;
    GSTEXTURE osk_tex;
    unsigned int rendered_osk_generation = 0;

    /*
     * D17AL-F4:
     * A local confirmation redraw does not consume or cancel the current
     * incremental FramebufferUpdateRequest.
     */
    int rfb_update_request_outstanding = 0;

    const u64 black =
        GS_SETREG_RGBAQ(0x00, 0x00, 0x00, 0x80, 0x00);

    const u64 tex_color =
        GS_SETREG_RGBAQ(0x80, 0x80, 0x80, 0x80, 0x00);

    /*
     * DMAKit remains alive across future display-mode reconstruction.
     * Preserve the proven one-time initialization ordering for E1-B.
     */
    dmaKit_init(
        D_CTRL_RELE_OFF,
        D_CTRL_MFD_OFF,
        D_CTRL_STS_UNSPEC,
        D_CTRL_STD_OFF,
        D_CTRL_RCYC_8,
        1 << DMA_CHANNEL_GIF
    );

    dmaKit_chan_init(DMA_CHANNEL_GIF);

    gsGlobal = ps2vnc_display_create();
    if (gsGlobal == NULL) {
        screenf("ERROR: display initialization failed");
        return -1;
    }

    /*
     * TEST13J-A:
     * Determine the centered area the television actually exposes before
     * publishing the VNC desktop. No VNC geometry changes are made yet.
     */
    if (!ps2vnc_display_config_loaded ||
        force_screen_fit_calibration) {

        if (run_screen_fit_calibration(
                gsGlobal,
                black,
                1,
                NULL) < 0)
            return -1;

        /*
         * Startup override is one-shot.  A later reconnect in this same
         * session must not unexpectedly reopen calibration.
         */
        force_screen_fit_calibration = 0;
    }

    /*
     * TEST13J-D3-A2:
     *
     * Keep the physical GS output at 1280x720, but make TigerVNC itself
     * render only the guaranteed-visible calibrated desktop.
     */
    {
        unsigned int target_x;
        unsigned int target_y;
        unsigned int target_w;
        unsigned int target_h;
        unsigned int logical_w;
        unsigned int logical_h;

        if (calculate_presented_desktop_geometry(
                &target_x,
                &target_y,
                &target_w,
                &target_h) < 0) {

            screenf("ERROR: invalid calibrated desktop geometry");
            return -1;
        }

        /*
         * D2 already proved this SetDesktopSize + 1x1 acknowledgement
         * path. Resize only when the server is not already at the target.
         */
        if (ps2vnc_display_logical_desktop_size(
                target_w, target_h,
                &logical_w, &logical_h) < 0)
            return -1;

        if (desktop_width != logical_w ||
            desktop_height != logical_h) {

            if (request_and_confirm_desktop_size(
                    sock,
                    rfb_framebuffer,
                    gs_framebuffer,
                    logical_w,
                    logical_h) < 0) {

                return -1;
            }
        }

        /*
         * From this point onward the RFB framebuffer is tightly packed at
         * the backend-selected logical desktop width. HIRES uses the
         * calibrated width; Standard uses its fixed framebuffer width.
         */
        desktop_width = logical_w;
        desktop_height = logical_h;

        desktop_output_x = target_x;
        desktop_output_y = target_y;
        desktop_output_width = target_w;
        desktop_output_height = target_h;

        debug_emit_geometry_snapshot();

        desktop_geometry_locked = 1;

        /*
         * Stop requesting EDS during normal rendering and obtain a clean
         * complete Raw framebuffer at the NEW desktop geometry.
         *
         * Unlike D2/A1, there is deliberately NO restore to 1280x720.
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
                1) < 0) {

            return -1;
        }
    }

    if (ps2vnc_display_prepare_desktop(
            gsGlobal, &tex, gs_framebuffer) < 0) {
        screenf("ERROR: HIRES presentation setup failed");
        return -1;
    }

    if (set_live_encoding(sock) < 0) {
        screenf("ERROR: live SetEncodings failed");
        return -1;
    }

    ps2vnc_display_prepare_osk(gsGlobal, &osk_tex);

    /*
     * H4C3C3C:
     * The complete runtime display now exists.  Only at this boundary may
     * startup recovery promote B or mark restored A durable.
     */
    if (ps2vnc_startup_finalize_display_transaction() < 0) {
        screenf("DISPLAY TX: startup finalization failed");
        return -1;
    }

    /*
     * The video path below remains the known-good Milestone 6 blocking
     * incremental loop.  Controller input runs independently.
     */
    debug_stage_id = DBG_STAGE_CONTROLLER_START;

    rfb_connected = 1;

    if (start_controller_thread(sock) < 0) {
        rfb_connected = 0;
        return -1;
    }

    for (;;) {
        if (exit_requested)
            return 1;

        debug_frame_counter++;
        debug_stage_id = DBG_STAGE_LOOP_BEGIN;
        debug_changed_bytes = 0;

        /*
         * Remote control is serviced at complete main-loop boundaries and
         * also from the idle EAGAIN path. Lease expiration is PS2-owned.
         */
        (void)ps2vnc_remote_display_lease_expired();
        (void)ps2vnc_remote_control_poll_idle();

        /*
         * TEST15E3-E:
         * Manual Refresh can arrive between RFB transactions.  If it arrives
         * inside recv_exact(), that function interrupts the suspect stream and
         * the receive-error branch below performs this same transaction.
         */
        if (runtime_action_request ==
            RUNTIME_ACTION_REFRESH_RFB) {

            int recovered_sock;

            debug_emit_now();

            recovered_sock =
                perform_manual_rfb_recovery(
                    sock,
                    rfb_framebuffer,
                    gs_framebuffer,
                    gsGlobal,
                    &tex,
                    &osk_tex
                );

            if (recovered_sock < 0) {
                if (exit_requested)
                    return 1;

                return -1;
            }

            sock = recovered_sock;

            rendered_osk_generation =
                osk_generation - 1;

            continue;
        }

        /*
         * H4B2C:
         * Controller X publishes only an acknowledgement request.  The main
         * RFB/render thread owns the management socket and releases this UI
         * only after the Pi confirms ACK with HTTP 200.
         */
        if (display_restore_ack_visible &&
            display_restore_ack_request) {

            display_restore_ack_request = 0;

            if (ps2vnc_display_transaction_ack_to_pi() < 0) {
                screenf("DISPLAY TX: ACK failed; retry with X");

                osk_generation++;
                rendered_osk_generation =
                    osk_generation - 1;
                continue;
            }

            screenf(
                "DISPLAY TX: ACK complete %s",
                display_restore_ack_mode ?
                    display_restore_ack_mode->name : "?");

            /*
             * Withdraw visible ownership before reopening Display Modes.
             */
            display_restore_ack_visible = 0;
            display_restore_ack_mode = NULL;

            display_confirm_return_to_active_menu();

            rendered_osk_generation =
                osk_generation - 1;
            continue;
        }

        /*
         * TEST15E3K A14-G2-D2B:
         * Same-mode calibration is a main-thread transaction.  Consume it at
         * the same proven between-RFB boundary used by runtime mode switching,
         * before the next incremental framebuffer request can be issued.
         */

        /*
         * D17AL-F2:
         *
         * Display admission/UI policy uses the same main-thread management
         * ownership boundary as calibration and mode transactions.
         */
        if (display_policy_request !=
                DISPLAY_POLICY_REQUEST_NONE) {

            int request =
                display_policy_request;

            int selection =
                display_policy_request_selection;

            int value =
                display_policy_request_value;

            int policy_result = -1;

            if (request ==
                    DISPLAY_POLICY_REQUEST_REFRESH) {

                policy_result =
                    ps2vnc_display_policy_refresh_from_pi();

            } else if (
                request ==
                    DISPLAY_POLICY_REQUEST_LOCK) {

                if (display_policy_loaded &&
                    selection >= 0 &&
                    (unsigned int)selection <
                        VIDEO_MODE_COUNT &&
                    video_modes[selection] !=
                        active_video_mode) {

                    policy_result =
                        ps2vnc_display_policy_lock_to_pi(
                            selection,
                            value);

                    if (policy_result == 0)
                        display_mode_locked[selection] =
                            value ? 1 : 0;
                }

            } else if (
                request ==
                    DISPLAY_POLICY_REQUEST_HIDE_LOCKED) {

                if (display_policy_loaded) {
                    policy_result =
                        ps2vnc_display_policy_hide_to_pi(
                            value);

                    if (policy_result == 0)
                        display_hide_locked_modes =
                            value ? 1 : 0;
                }
            }

            if (policy_result < 0) {
                display_policy_error = 1;

            } else {
                display_policy_error = 0;
            }

            display_policy_request_selection = -1;
            display_policy_request_value = 0;
            display_policy_request =
                DISPLAY_POLICY_REQUEST_NONE;

            display_menu_normalize_selection();

            osk_generation++;
            rendered_osk_generation =
                osk_generation - 1;

            continue;
        }


        if (display_calibration_request != DISPLAY_MODE_REQUEST_NONE) {
            int request = display_calibration_request;
            const ps2vnc_video_mode_t *target_mode;

            display_calibration_request = DISPLAY_MODE_REQUEST_NONE;
            target_mode = display_mode_request_target(request);

            /*
             * The request is valid only while it still names the active mode.
             * Never reinterpret a stale calibration request as a mode switch.
             */
            if (target_mode == NULL ||
                target_mode != active_video_mode) {

                display_menu_selection =
                    display_menu_selection_for_mode(active_video_mode);
                display_menu_error = 1;
                display_menu_visible = 1;
                display_menu_parent_system = 0;
                osk_generation++;
                rendered_osk_generation = osk_generation - 1;
                continue;
            }

            if (ps2vnc_run_runtime_calibration(
                    sock,
                    rfb_framebuffer,
                    gs_framebuffer,
                    gsGlobal,
                    &tex,
                    black) < 0) {

                return -1;
            }

            /*
             * Return directly to the active row so X once again means
             * "Calibrate <mode> Safe Area".
             */
            display_menu_selection =
                display_menu_selection_for_mode(active_video_mode);
            display_menu_error = 0;
            display_menu_visible = 1;
            display_menu_parent_system = 0;
            osk_generation++;
            rendered_osk_generation = osk_generation - 1;
            continue;
        }

        /*
         * H3C2B1: a provisional display owns this main-loop transaction until
         * the user explicitly keeps it or rollback reconstructs known-good.
         */
        if (display_confirm_pending &&
            display_confirm_action == DISPLAY_CONFIRM_ACTION_KEEP) {

            /*
             * H4B2A:
             * B is already the complete active runtime profile. KEEP does
             * not reconstruct anything. It only asks the Pi to promote the
             * already-running provisional transaction to confirmed state.
             */
            if (ps2vnc_display_transaction_commit_to_pi() < 0) {

                /*
                 * Do not silently accept a mode whose global persistence
                 * commit was not acknowledged. Return ownership to the
                 * confirmation UI so the user may explicitly retry.
                 */
                screenf("DISPLAY TX: commit failed");

                display_confirm_action =
                    DISPLAY_CONFIRM_ACTION_NONE;
                display_confirm_selection =
                    DISPLAY_CONFIRM_KEEP;
                display_confirm_visible = 1;

                osk_generation++;
                rendered_osk_generation =
                    osk_generation - 1;
                continue;
            }

            screenf(
                "DISPLAY TX: committed %s",
                active_video_mode->name);

            display_confirm_disarm_transaction();
            display_confirm_return_to_active_menu();
            rendered_osk_generation = osk_generation - 1;
            continue;
        }

        if (display_confirm_pending &&
            display_confirm_action == DISPLAY_CONFIRM_ACTION_ROLLBACK) {

            int rollback_sock =
                ps2vnc_global_rollback_display_transaction(
                    sock,
                    0,
                    rfb_framebuffer,
                    gs_framebuffer,
                    &gsGlobal,
                    &tex,
                    &osk_tex);

            if (rollback_sock < 0)
                return -1;

            sock = rollback_sock;

            /*
             * F8J2-B3B:
             *
             * A successful rollback reconstruction is an RFB request-epoch
             * boundary exactly like a successful forward mode switch.
             *
             * Explicit remote restore can arrive through the benign
             * RFB_RECEIVE_LOCAL_REDRAW path.  That path correctly preserves
             * the PRE-ROLLBACK incremental request while yielding to the
             * transaction owner, but the request cannot remain authoritative
             * after ps2vnc_global_rollback_display_transaction() has consumed
             * its own EDS/full-frame reconstruction of the previous mode.
             *
             * Force the first restored live iteration to issue a fresh
             * incremental FramebufferUpdateRequest for the restored geometry.
             */
            rfb_update_request_outstanding = 0;

            display_confirm_return_to_active_menu();
            rendered_osk_generation = osk_generation - 1;
            continue;
        }

        /*
         * TEST15E2-D2B:
         * Consume local controller requests on the main RFB/render thread.
         *
         * Target configuration and geometry are resolved before entering
         * ps2vnc_switch_display_mode(), so an unavailable/invalid preset
         * leaves the current GS display completely untouched.
         */
        if (display_mode_request != DISPLAY_MODE_REQUEST_NONE) {
            int request = display_mode_request;
            int request_is_remote =
                display_mode_request_remote;
            int display_switch_result;
            const ps2vnc_video_mode_t *target_mode = NULL;
            ps2vnc_display_config_t target_config;
            unsigned int target_x;
            unsigned int target_y;
            unsigned int target_w;
            unsigned int target_h;

            ps2vnc_display_profile_t previous_profile;
            ps2vnc_display_profile_t candidate_profile;

            /*
             * Claim the request before doing any blocking work.  The
             * controller may publish a later request only after the user
             * opens the selector again.
             */
            display_mode_request = DISPLAY_MODE_REQUEST_NONE;
            display_mode_request_remote = 0;

            target_mode =
                display_mode_request_target(request);

            /*
             * Same-mode requests are defined as harmless no-ops.
             * The controller normally filters these already.
             */
            if (target_mode == active_video_mode) {
                if (request_is_remote)
                    ps2vnc_remote_display_fail(4);

                continue;
            }


            /*
             * Lock policy is checked again at the sole GS-switch owner.
             * A stale or malformed controller request therefore still cannot
             * construct a locked candidate.
             */
            if (target_mode == NULL ||
                (!request_is_remote &&
                 (!display_policy_loaded ||
                  display_mode_locked[
                    display_menu_selection_for_mode(
                        target_mode)]))) {

                display_menu_selection =
                    display_menu_selection_for_mode(
                        target_mode != NULL
                            ? target_mode
                            : active_video_mode);

                display_menu_tab =
                    display_menu_tab_for_mode(
                        target_mode != NULL
                            ? target_mode
                            : active_video_mode);

                display_menu_error = 1;
                display_menu_visible = 1;
                display_menu_parent_system = 0;

                osk_generation++;
                rendered_osk_generation =
                    osk_generation - 1;

                continue;
            }

            if (target_mode == NULL ||
                !ps2vnc_config_get_display_for_mode(
                    target_mode, &target_config) ||
                ps2vnc_config_presented_geometry_for_mode(
                    target_mode,
                    &target_config,
                    &target_x,
                    &target_y,
                    &target_w,
                    &target_h) < 0) {

                if (request_is_remote) {
                    ps2vnc_remote_display_fail(5);
                    continue;
                }

                display_menu_selection =
                    display_menu_selection_for_mode(target_mode);
                display_menu_error = 1;
                display_menu_visible = 1;
                osk_generation++;
                rendered_osk_generation = osk_generation - 1;
                continue;
            }

            /*
             * Build both complete cross-machine profiles while A is still
             * unquestionably active. No runtime or persistence state has
             * changed yet.
             */
            if (ps2vnc_display_profile_capture_active(
                    &previous_profile) < 0 ||
                ps2vnc_display_profile_resolve_candidate(
                    target_mode,
                    target_x,
                    target_y,
                    target_w,
                    target_h,
                    &candidate_profile) < 0) {

                if (request_is_remote) {
                    ps2vnc_remote_display_fail(6);
                    continue;
                }

                screenf("DISPLAY TX: profile resolution failed");

                display_menu_selection =
                    display_menu_selection_for_mode(
                        active_video_mode);
                display_menu_error = 1;
                display_menu_visible = 1;

                osk_generation++;
                rendered_osk_generation =
                    osk_generation - 1;
                continue;
            }

            /*
             * Global transaction record FIRST.
             *
             * If BEGIN is refused or the management service is unavailable,
             * A remains fully active on both machines and no risky display
             * reconstruction begins.
             */
            if (request_is_remote) {
                remote_display_state =
                    REMOTE_DISPLAY_SWITCHING;

                remote_display_error_code = 0;

                (void)ps2vnc_remote_result_post(
                    "SWITCHING",
                    0);
            }

            if (ps2vnc_display_transaction_begin_to_pi(
                    &previous_profile,
                    &candidate_profile) < 0) {

                screenf("DISPLAY TX: begin failed");

                if (request_is_remote) {
                    ps2vnc_remote_display_fail(7);
                    continue;
                }

                display_menu_selection =
                    display_menu_selection_for_mode(
                        active_video_mode);
                display_menu_error = 1;
                display_menu_visible = 1;

                osk_generation++;
                rendered_osk_generation =
                    osk_generation - 1;
                continue;
            }

            /*
             * The Pi now durably knows A is confirmed and B is provisional.
             * Arm the local timeout/rollback obligation before touching GS
             * or RFB runtime state.
             */
            display_confirm_arm_transaction(
                active_video_mode,
                desktop_output_x,
                desktop_output_y,
                desktop_output_width,
                desktop_output_height,
                screen_fit_width,
                screen_fit_height,
                screen_fit_offset_x,
                screen_fit_offset_y,
                screen_fit_full_bypass);

            display_transition_switch_in_progress = 1;

            display_switch_result =
                ps2vnc_switch_display_mode(
                    sock,
                    rfb_framebuffer,
                    gs_framebuffer,
                    &gsGlobal,
                    &tex,
                    &osk_tex,
                    target_mode,
                    target_x,
                    target_y,
                    target_w,
                    target_h);

            /*
             * F8J2-B3A:
             *
             * The synchronous display reconstruction owns its own EDS/full
             * framebuffer requests and consumes the authoritative candidate
             * frame before returning.
             *
             * A remote display request may have arrived by yielding from an
             * older incremental FramebufferUpdateRequest at a clean message
             * boundary.  That caller-local request bookkeeping belongs to the
             * PRE-SWITCH geometry and must not survive reconstruction.
             *
             * Re-arm live incremental traffic unconditionally after the
             * synchronous switch attempt.  Success must send a fresh request
             * for the new geometry; failure/rollback must likewise never
             * inherit the abandoned pre-switch request.
             */
            rfb_update_request_outstanding = 0;


            /*
             * Clear this before inspecting the result.  Once the synchronous
             * call has returned, all failures and elapsed deadlines belong to
             * the existing graceful main-thread rollback machinery.
             */
            display_transition_switch_in_progress = 0;

            if (display_switch_result < 0) {

                int rollback_sock;

                if (request_is_remote) {
                    unsigned int diagnostic_live =
                        live_error_code > 999
                            ? 999u
                            : (unsigned int)live_error_code;

                    remote_display_error_code =
                        800000u +
                        display_switch_failure_stage * 1000u +
                        diagnostic_live;
                }

                /*
                 * Candidate failed or was interrupted by blind-X/timeout.
                 * Never leave the partially applied configuration active.
                 */
                display_confirm_action =
                    DISPLAY_CONFIRM_ACTION_ROLLBACK;

                rollback_sock =
                    ps2vnc_global_rollback_display_transaction(
                        sock,
                        1,
                    rfb_framebuffer,
                        gs_framebuffer,
                        &gsGlobal,
                        &tex,
                        &osk_tex);

                if (rollback_sock < 0)
                    return -1;

                sock = rollback_sock;

                display_confirm_return_to_active_menu();
                rendered_osk_generation = osk_generation - 1;
                continue;
            }

            /*
             * D17AL-F3:
             * The GS/RFB candidate is now complete. Publish the already-
             * validated mode-specific calibration into local screen-fit
             * authority before exposing B to confirmation.
             *
             * target_config was fetched and validated while A was still
             * active. Any unexpected publication mismatch is treated exactly
             * like a candidate failure and rolls back the complete profile.
             */
            {
                unsigned int verify_x;
                unsigned int verify_y;
                unsigned int verify_w;
                unsigned int verify_h;

                if (!ps2vnc_config_apply_display(&target_config) ||
                    calculate_presented_desktop_geometry(
                        &verify_x,
                        &verify_y,
                        &verify_w,
                        &verify_h) < 0 ||
                    verify_x != target_x ||
                    verify_y != target_y ||
                    verify_w != target_w ||
                    verify_h != target_h) {

                    int rollback_sock;

                    if (request_is_remote)
                        remote_display_error_code = 9;

                    screenf(
                        "DISPLAY TX: candidate calibration apply failed");

                    display_confirm_action =
                        DISPLAY_CONFIRM_ACTION_ROLLBACK;

                    rollback_sock =
                        ps2vnc_global_rollback_display_transaction(
                            sock,
                            0,
                            rfb_framebuffer,
                            gs_framebuffer,
                            &gsGlobal,
                            &tex,
                            &osk_tex);

                    if (rollback_sock < 0)
                        return -1;

                    sock = rollback_sock;

                    display_confirm_return_to_active_menu();
                    rendered_osk_generation =
                        osk_generation - 1;
                    continue;
                }
            }

            if (request_is_remote) {
                remote_display_state =
                    REMOTE_DISPLAY_ACTIVE_TEMP;

                remote_display_lease_started_tick =
                    GetTimerSystemTime();

                /*
                 * Keep the durable transaction provisional. Do not expose
                 * KEEP/COMMIT confirmation UI and do not mutate startup_mode.
                 */
                display_confirm_visible = 0;
                display_confirm_seconds = 0;

                (void)ps2vnc_remote_result_post(
                    "ACTIVE_TEMP",
                    0);

                screenf(
                    "REMOTE DISPLAY: ACTIVE_TEMP %s tx=%u lease=%us",
                    active_video_mode->name,
                    remote_display_txid,
                    remote_display_lease_seconds);

                continue;
            }

            /*
             * The candidate is fully reconstructed but remains provisional.
             * The original 30-second deadline has continued running during
             * the switch itself.
             */
            display_confirm_seconds =
                display_confirm_remaining_seconds();

            if (display_confirm_seconds == 0 ||
                display_confirm_action ==
                    DISPLAY_CONFIRM_ACTION_ROLLBACK) {

                int rollback_sock;

                display_confirm_action =
                    DISPLAY_CONFIRM_ACTION_ROLLBACK;

                rollback_sock =
                    ps2vnc_global_rollback_display_transaction(
                        sock,
                        0,
                    rfb_framebuffer,
                        gs_framebuffer,
                        &gsGlobal,
                        &tex,
                        &osk_tex);

                if (rollback_sock < 0)
                    return -1;

                sock = rollback_sock;

                display_confirm_return_to_active_menu();
                rendered_osk_generation = osk_generation - 1;
                continue;
            }

            display_confirm_selection =
                DISPLAY_CONFIRM_GO_BACK;
            display_confirm_visible = 1;

            system_menu_visible = 0;
            display_menu_visible = 0;
            osk_visible = 0;

            osk_generation++;
            rendered_osk_generation = osk_generation - 1;
            continue;
        }

        if ((debug_frame_counter % 60) == 0) {
            debug_emit_now();
            profile_emit_now();
        }

        debug_stage_id = DBG_STAGE_PRE_DRAW;

        /*
         * HIRES already has its desktop background configured. Standard
         * instead clears and draws its conventional desktop texture here.
         */
        if (ps2vnc_display_draw_desktop(
                gsGlobal,
                &tex,
                black,
                tex_color) < 0) {
            live_error_code = 208;
            return -1;
        }

        /*
         * Render/upload the OSK texture only when its local state changed.
         * Drawing it is just one additional textured sprite.
         */
        if (osk_visible ||
            display_menu_visible ||
            system_menu_visible ||
            display_confirm_visible ||
            display_restore_ack_visible) {

            if (rendered_osk_generation != osk_generation) {
                /*
                 * Display confirmation is safety-critical and owns the
                 * overlay whenever visible. It must not be hidden beneath
                 * the ordinary system/display menus or OSK.
                 */
                if (display_restore_ack_visible)
                    display_restore_ack_render_bitmap();
                else if (display_confirm_visible)
                    display_confirm_render_bitmap();
                else if (system_menu_visible)
                    system_menu_render_bitmap();
                else if (display_menu_visible)
                    display_menu_render_bitmap();
                else
                    osk_render_bitmap();

                /*
                 * Prevent an immediate OSK texture upload from racing
                 * the previous HIRES frame.
                 */
                if (ps2vnc_display_wait_texture_idle(gsGlobal) < 0)
                    return -1;

                gsKit_texture_upload(gsGlobal, &osk_tex);

                rendered_osk_generation = osk_generation;
            }

            {
                float dst_x0;
                float dst_y0;
                float dst_x1;
                float dst_y1;

                {
                    float safe_x0 = 0.0f;
                    float safe_y0 = 0.0f;
                    float safe_w = (float)OUTPUT_WIDTH;
                    float safe_h = (float)OUTPUT_HEIGHT;

                    float scale_x;
                    float scale_y;
                    float scale;
                    float dst_w;
                    float dst_h;

                    /*
                     * D17AL-F1:
                     *
                     * Ordinary local PS2VNC UI is contained by the calibrated
                     * safe desktop. Calibration itself is intentionally drawn
                     * directly against the physical raster elsewhere.
                     */
                    if (desktop_geometry_locked &&
                        region_fits_u32(
                            desktop_output_x,
                            desktop_output_y,
                            desktop_output_width,
                            desktop_output_height,
                            OUTPUT_WIDTH,
                            OUTPUT_HEIGHT)) {

                        safe_x0 = (float)desktop_output_x;
                        safe_y0 = (float)desktop_output_y;
                        safe_w = (float)desktop_output_width;
                        safe_h = (float)desktop_output_height;
                    }

                    scale_x =
                        (safe_w - 12.0f) /
                        (float)OSK_WIDTH;

                    scale_y =
                        (safe_h - 12.0f) /
                        (float)OSK_HEIGHT;

                    scale =
                        (scale_x < scale_y) ?
                        scale_x : scale_y;

                    /*
                     * Menus can grow to use the safe desktop.
                     * The normal keyboard remains 1:1 unless it must shrink.
                     */
                    if (!(display_menu_visible ||
                          system_menu_visible ||
                          display_confirm_visible ||
                          display_restore_ack_visible) &&
                        scale > 1.0f) {

                        scale = 1.0f;
                    }

                    if (scale < 0.25f)
                        scale = 0.25f;

                    dst_w = (float)OSK_WIDTH * scale;
                    dst_h = (float)OSK_HEIGHT * scale;

                    dst_x0 =
                        safe_x0 +
                        (safe_w - dst_w) / 2.0f;

                    if (display_menu_visible ||
                        system_menu_visible ||
                        display_confirm_visible ||
                        display_restore_ack_visible) {

                        dst_y0 =
                            safe_y0 +
                            (safe_h - dst_h) / 2.0f;

                    } else {

                        dst_y0 =
                            safe_y0 +
                            safe_h -
                            dst_h -
                            6.0f;
                    }

                    dst_x1 = dst_x0 + dst_w;
                    dst_y1 = dst_y0 + dst_h;
                }

                gsKit_prim_sprite_texture(
                    gsGlobal,
                    &osk_tex,
                    ps2vnc_display_output_x(dst_x0),
                    ps2vnc_display_output_y(dst_y0),
                    0.0f, 0.0f,
                    ps2vnc_display_output_x(dst_x1),
                    ps2vnc_display_output_y(dst_y1),
                    (float)OSK_WIDTH,
                    (float)OSK_HEIGHT,
                    2,
                    tex_color
                );
            }
        } else {
            rendered_osk_generation = osk_generation;
        }

        debug_stage_id = DBG_STAGE_POST_DRAW;

        /*
         * TEST13E HIRES presentation.
         *
         * Preserve the logical DRAW -> FLIP ordering, but HIRES performs
         * the actual rendering in scanline-driven passes.
         *
         * Legacy debug stage IDs are retained intentionally.
         */
        debug_stage_id = DBG_STAGE_PRE_QUEUE;
        {
            u64 t0 = GetTimerSystemTime();

            if (ps2vnc_display_finish_draw(gsGlobal) < 0)
                return -1;

            profile_present_sync_ticks += GetTimerSystemTime() - t0;
        }
        debug_stage_id = DBG_STAGE_POST_QUEUE;

        debug_stage_id = DBG_STAGE_PRE_FLIP;
        {
            u64 t0 = GetTimerSystemTime();

            if (ps2vnc_display_flip(gsGlobal) < 0)
                return -1;

            profile_flip_ticks += GetTimerSystemTime() - t0;
        }
        debug_stage_id = DBG_STAGE_POST_FLIP;

        /*
         * After the initial complete frame, request only pixels that changed.
         * Queued controller traffic is flushed before this main-thread request.
         */
        if (controller_thread_error) {
            debug_stage_id = DBG_STAGE_CONTROLLER_ERROR;
            live_error_code = 201;
            debug_emit_now();
            screenf("LIVE ERR 201 controller");
            return -1;
        }

        {
            u64 t0 = GetTimerSystemTime();
            if (ps2vnc_display_wait_presentation_memory_idle(
                    gsGlobal) < 0)
                return -1;

            profile_dirty_wait_ticks += GetTimerSystemTime() - t0;
        }

        debug_stage_id = DBG_STAGE_PRE_REQUEST;
        profile_rfb_start_tick = GetTimerSystemTime();
        /*
         * TEST15D3-A3: both presentation buffers are kept coherent after
         * every completed update, so normal live traffic can be incremental.
         */
        if (!rfb_update_request_outstanding) {
        if (request_framebuffer(sock, 1) < 0) {
            int recovered_sock;

            /*
             * An unconfirmed display owns transport failure as part of its
             * transaction. Abandon the candidate instead of generically
             * recovering into an unapproved display configuration.
             *
             * KEEP is excluded: once explicitly chosen, an unrelated RFB
             * failure no longer rejects the selected display mode.
             */
            if (display_confirm_pending &&
                display_confirm_action != DISPLAY_CONFIRM_ACTION_KEEP) {

                display_confirm_action =
                    DISPLAY_CONFIRM_ACTION_ROLLBACK;

                recovered_sock =
                    ps2vnc_global_rollback_display_transaction(
                        sock,
                        1,
                    rfb_framebuffer,
                        gs_framebuffer,
                        &gsGlobal,
                        &tex,
                        &osk_tex);

                if (recovered_sock < 0)
                    return -1;

                sock = recovered_sock;

                display_confirm_return_to_active_menu();
                rendered_osk_generation = osk_generation - 1;
                continue;
            }

            debug_stage_id = DBG_STAGE_REQUEST_ERROR;
            live_error_code = 202;
            debug_emit_now();
            screenf("LIVE ERR 202 request");

            recovered_sock =
                recover_rfb_connection(
                    sock,
                    rfb_framebuffer,
                    gs_framebuffer,
                    gsGlobal,
                    &tex,
                    &osk_tex
                );

            if (recovered_sock < 0) {
                if (exit_requested)
                    return 1;

                return -1;
            }

            sock = recovered_sock;

            /*
             * Recovery status temporarily reused the OSK bitmap.
             * Force a fresh keyboard render if the OSK was open.
             */
            osk_generation++;
            rendered_osk_generation =
                osk_generation - 1;

            continue;
        }

            rfb_update_request_outstanding = 1;
        }

        if (ps2vnc_display_begin_update() < 0)
            return -1;

        debug_stage_id = DBG_STAGE_POST_REQUEST;

        {
            int changed_bytes;
            int direct_present_complete;

            debug_stage_id = DBG_STAGE_WAIT_RFB;

            /*
             * Snapshot the already-rendered confirmation generation.
             * Any later controller-side countdown/selection generation
             * change may yield at the NEXT server-message boundary.
             */
            rfb_confirm_wait_generation =
                rendered_osk_generation;

            rfb_confirm_live_yield_enabled =
                (display_confirm_pending &&
                 display_confirm_visible) ? 1 : 0;

            changed_bytes = receive_framebuffer_update(
                sock,
                rfb_framebuffer,
                gs_framebuffer,
                0,
                0
            );

            rfb_confirm_live_yield_enabled = 0;

            if (changed_bytes == RFB_RECEIVE_LOCAL_REDRAW) {
                /*
                 * No server-message byte was consumed, so there is no
                 * partial decoder state to finish. The original incremental
                 * request remains outstanding while the outer loop gets a
                 * chance to draw/flip the new confirmation bitmap.
                 */
                ps2vnc_display_direct_write_cancel();
                continue;
            }

            /*
             * Any real FramebufferUpdate completion or transport failure
             * consumes/invalidates the outstanding request contract.
             */
            rfb_update_request_outstanding = 0;

            direct_present_complete =
                ps2vnc_display_finish_update_decode();

            if (changed_bytes >= 0) {
                profile_rfb_ticks +=
                    GetTimerSystemTime() - profile_rfb_start_tick;
                profile_rfb_updates++;
                if (changed_bytes > 0)
                    profile_changed_bytes += (unsigned int)changed_bytes;
            }

            debug_changed_bytes = changed_bytes;
            debug_stage_id = DBG_STAGE_POST_RECEIVE;

            if (changed_bytes < 0) {
                /*
                 * recv_exact() deliberately aborts when blind-X/timeout
                 * publishes ROLLBACK. A genuine receive failure while the
                 * candidate is still awaiting confirmation has the same
                 * transaction semantics: restore known-good, not candidate.
                 */
                if (display_confirm_pending &&
                    display_confirm_action !=
                        DISPLAY_CONFIRM_ACTION_KEEP) {

                    int rollback_sock;

                    display_confirm_action =
                        DISPLAY_CONFIRM_ACTION_ROLLBACK;

                    rollback_sock =
                        ps2vnc_global_rollback_display_transaction(
                            sock,
                            1,
                    rfb_framebuffer,
                            gs_framebuffer,
                            &gsGlobal,
                            &tex,
                            &osk_tex);

                    if (rollback_sock < 0)
                        return -1;

                    sock = rollback_sock;

                    display_confirm_return_to_active_menu();
                    rendered_osk_generation = osk_generation - 1;
                    continue;
                }

                if (runtime_action_request ==
                    RUNTIME_ACTION_REFRESH_RFB) {

                    int recovered_sock =
                        perform_manual_rfb_recovery(
                            sock,
                            rfb_framebuffer,
                            gs_framebuffer,
                            gsGlobal,
                            &tex,
                            &osk_tex
                        );

                    if (recovered_sock < 0) {
                        if (exit_requested)
                            return 1;

                        return -1;
                    }

                    sock = recovered_sock;

                    rendered_osk_generation =
                        osk_generation - 1;

                    continue;
                }

                if (exit_requested)
                    return 1;

                debug_stage_id = DBG_STAGE_RECEIVE_ERROR;
                debug_emit_now();
                screenf("LIVE ERR %d type=%u",
                        live_error_code,
                        live_message_type);
                screenf("rect %u/%u enc=%08x",
                        live_rect_index,
                        live_rect_count,
                        live_encoding);
                screenf("xywh %u,%u %ux%u",
                        live_rect_x,
                        live_rect_y,
                        live_rect_w,
                        live_rect_h);
                screenf("hdr %02x%02x%02x%02x %02x%02x%02x%02x",
                        live_hdr_raw[0],
                        live_hdr_raw[1],
                        live_hdr_raw[2],
                        live_hdr_raw[3],
                        live_hdr_raw[4],
                        live_hdr_raw[5],
                        live_hdr_raw[6],
                        live_hdr_raw[7]);
                screenf("    %02x%02x%02x%02x hav=%u rn=%d",
                        live_hdr_raw[8],
                        live_hdr_raw[9],
                        live_hdr_raw[10],
                        live_hdr_raw[11],
                        live_hdr_start_avail,
                        live_hdr_recv_n);
                screenf("upd %02x%02x%02x%02x",
                        live_update_raw[0],
                        live_update_raw[1],
                        live_update_raw[2],
                        live_update_raw[3]);

                {
                    int recovered_sock =
                        recover_rfb_connection(
                            sock,
                            rfb_framebuffer,
                            gs_framebuffer,
                            gsGlobal,
                            &tex,
                            &osk_tex
                        );

                    if (recovered_sock < 0) {
                        if (exit_requested)
                            return 1;

                        return -1;
                    }

                    sock = recovered_sock;

                    osk_generation++;
                    rendered_osk_generation =
                        osk_generation - 1;

                    continue;
                }
            }

            /*
             * The RFB stream is already incremental. The active presentation
             * backend owns how this successfully completed changed update
             * becomes visible.
             */
            if (changed_bytes > 0 && !TEST29_DISCARD_LIVE_PIXELS) {
                debug_stage_id = DBG_STAGE_PRE_TEXTURE;

                if (ps2vnc_display_commit_update(
                        gsGlobal,
                        &tex,
                        direct_present_complete) < 0)
                    return -1;

                debug_stage_id = DBG_STAGE_POST_TEXTURE;
            }

        }
    }
}

int main(int argc, char *argv[])
{
    struct ip4_addr IP, NM, GW;
    int sock;
    unsigned short *rfb_framebuffer;
    unsigned short *gs_framebuffer;

    /*
     * TEST15B2E:
     * RFB is already requested in GS-compatible B5:G5:R5 ordering.
     * Use one authoritative EE framebuffer for both receive and GS-ready
     * storage instead of writing every changed pixel through two buffers.
     *
     * Keep the rfb_framebuffer alias temporarily so the proven connection,
     * resize and recovery plumbing remains unchanged for this experiment.
     */
    gs_framebuffer = (unsigned short *)memalign(
        128,
        VNC_WIDTH * VNC_HEIGHT * sizeof(unsigned short)
    );

    if (gs_framebuffer == NULL)
        return 1;

    rfb_framebuffer = gs_framebuffer;

    memset(gs_framebuffer, 0,
           VNC_WIDTH * VNC_HEIGHT * sizeof(unsigned short));

    /*
     * Keep the pre-screen startup unchanged; once debug output is available,
     * every subsequent potentially blocking step has a short marker.
     */
    sceSifInitRpc(0);
    while (!SifIopReset("", 0)) {}
    while (!SifIopSync()) {}

    sceSifInitRpc(0);
    SifLoadFileInit();
    SifInitIopHeap();
    sbv_patch_enable_lmb();

    SifExecModuleBuffer(DEV9_irx, size_DEV9_irx, 0, NULL, NULL);
    SifExecModuleBuffer(NETMAN_irx, size_NETMAN_irx, 0, NULL, NULL);
    SifExecModuleBuffer(SMAP_irx, size_SMAP_irx, 0, NULL, NULL);

    NetManInit();

    init_scr();
    scr_setCursor(0);
    screen_row = 0;

    screenf("TEST31 RX SENTINEL TRACE");
    screenf("Hold L1 + R1 to force display calibration");

    screenf("PAD MOD 1");
    if (SifLoadModule("rom0:XSIO2MAN", 0, NULL) < 0) {
        screenf("ERR XSIO2");
        goto wait;
    }

    screenf("PAD MOD 2");
    if (SifLoadModule("rom0:XPADMAN", 0, NULL) < 0) {
        screenf("ERR XPAD");
        goto wait;
    }

    screenf("PAD INIT");
    if (padInit(0) != 1) {
        screenf("ERR PADINIT");
        goto wait;
    }

    screenf("PAD OPEN");
    if (padPortOpen(0, 0, pad_area) == 0) {
        screenf("ERR PADOPEN");
        goto wait;
    }

    /*
     * Test11K-B:
     * Force DualShock analog mode so L3/R3 and true analog coordinates
     * are available without requiring the user to press the controller's
     * ANALOG button manually.
     *
     * MMODE_LOCK asks the pad to remain in this mode.
     */
    screenf("PAD ANALOG");

    if (wait_pad_ready_after_mode_change() < 0) {
        screenf("ERR PAD READY");
        goto wait;
    }

    if (padSetMainMode(
            0,
            0,
            PAD_MMODE_DUALSHOCK,
            PAD_MMODE_LOCK) == 0) {
        screenf("ERR PAD MODE");
        goto wait;
    }

    if (wait_pad_ready_after_mode_change() < 0) {
        screenf("ERR PAD ANALOG WAIT");
        goto wait;
    }

    screenf("PAD OK");

    force_screen_fit_calibration =
        startup_force_calibration_requested();

    if (force_screen_fit_calibration)
        screenf("L1+R1: FORCE CALIBRATION");

    IP4_ADDR(&IP, 192, 168, 50, 2);
    IP4_ADDR(&NM, 255, 255, 255, 0);
    IP4_ADDR(&GW, 192, 168, 50, 1);

    screenf("TCP INIT");

    if (ps2ipInit(&IP, &NM, &GW) < 0) {
        screenf("ERROR: ps2ipInit() failed");
        goto wait;
    }

    screenf("Waiting for Ethernet link...");

    if (ethWaitValidNetIFLinkState() != 0) {
        screenf("Unable to establish Ethernet connection");
        screenf("Returning to system menu...");
        goto exit_to_osdsys;
    }

    screenf("Ethernet link is UP");

    /*
     * TEST13J-D4-A3:
     * Retrieve the general PS2VNC configuration before opening RFB.
     *
     * Failure is intentionally non-fatal.  Without a valid stored display
     * preset, the existing interactive calibration path remains authoritative.
     */
    ps2vnc_config_load_from_pi();

    /*
     * H4C3B2B:
     * Durable display authority must be known before ordinary RFB startup.
     * A pending transaction may imply that TigerVNC is still at a geometry
     * incompatible with the confirmed startup profile.
     */
    if (ps2vnc_display_transaction_startup_preflight() < 0) {
        screenf("DISPLAY TX: unsafe startup blocked");
        screenf("Returning to system menu...");
        goto exit_to_osdsys;
    }

    if (ps2vnc_startup_prepare_display_transaction() < 0) {
        screenf("DISPLAY TX: startup recovery preparation failed");
        screenf("Returning to system menu...");
        goto exit_to_osdsys;
    }

    if (ps2vnc_startup_apply_authoritative_profile() < 0) {
        screenf("DISPLAY TX: startup profile apply failed");
        screenf("Returning to system menu...");
        goto exit_to_osdsys;
    }

    debug_stage_id = DBG_STAGE_NET_READY;
    if (debug_udp_init() < 0)
        screenf("WARN: debug UDP did not initialize");
    else
        debug_emit_now();

    rfb_rx_pos = 0;
    rfb_rx_end = 0;
    rfb_rx_syscalls = 0;
    rfb_last_recv_n = 0;
    rfb_last_recv_call = 0;
    memset((void *)live_hdr_raw, 0, sizeof(live_hdr_raw));
    memset((void *)live_update_raw, 0, sizeof(live_update_raw));

    screenf("Establishing VNC session...");

    sock = ps2vnc_startup_connect_reconciled(
        rfb_framebuffer,
        gs_framebuffer
    );

    if (sock < 0) {
        screenf("Unable to establish VNC session");
        screenf("Returning to system menu...");
        goto exit_to_osdsys;
    }

    screenf("FULL FRAMEBUFFER RECEIVED");
    screenf("Starting incremental gsKit display...");

    live_error_code = 0;
    live_message_type = 0;
    live_rect_count = 0;
    live_rect_index = 0;
    live_rect_x = 0;
    live_rect_y = 0;
    live_rect_w = 0;
    live_rect_h = 0;
    live_encoding = 0;

    {
        int live_result = display_live_desktop(
            sock,
            rfb_framebuffer,
            gs_framebuffer);

        if (live_result > 0)
            goto exit_to_osdsys;

        if (live_result < 0)
            goto exit_to_osdsys;
    }

exit_to_osdsys:
    screenf("Returning to system menu...");
    LoadExecPS2("rom0:OSDSYS", 0, NULL);

    /*
     * LoadExecPS2 normally never returns.  If it does, fall back to the
     * existing stopped state so we get visible evidence instead of hanging
     * somewhere unknown.
     */
    screenf("ERROR: OSDSYS return failed");

wait:
    screenf("Stopped - reset PS2 when done");
    SleepThread();

    return 0;
}

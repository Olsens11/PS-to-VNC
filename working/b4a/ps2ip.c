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
ps2vnc_display_transaction_t startup_display_transaction;

int startup_display_transaction_loaded = 0;

/*
 * H4C3C1:
 * The confirmed profile is derived from the SAME configuration body that
 * startup applied. Do not re-fetch configuration later to reconstruct it.
 */
ps2vnc_display_profile_t startup_confirmed_display_profile;

int startup_confirmed_display_profile_loaded = 0;

ps2vnc_display_profile_t startup_authoritative_display_profile;

int startup_authoritative_display_profile_loaded = 0;

/*
 * Durable-state obligations retained for the later reconciliation stage.
 *
 * provisional: RESTORE -> repair A -> RESTORED -> ACK
 * restoring:              repair A -> RESTORED -> ACK
 * restored:                          verify A -> ACK
 */
int startup_display_commit_finish_required = 0;
int startup_display_restore_required = 0;
int startup_display_restored_mark_required = 0;
int startup_display_restore_ack_required = 0;

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

int ps2vnc_video_mode_supports_backend(
    const ps2vnc_video_mode_t *mode,
    ps2vnc_backend_t backend)
{
    if (mode == NULL || backend == PS2VNC_BACKEND_NONE)
        return 0;

    return (mode->allowed_backends & (unsigned int)backend) != 0;
}

/*
 * M3 coarse-first migration scaffold.
 *
 * The complete implementation envelope remains in this exact
 * translation unit. Later M3 steps recursively subdivide it.
 */
#include "ps2vnc_cross_types.h"
#include "ps2vnc_services_exports.h"
#include "ps2vnc_macro_body.inc"

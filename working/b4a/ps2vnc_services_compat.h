#ifndef PS2VNC_SERVICES_COMPAT_H
#define PS2VNC_SERVICES_COMPAT_H

/*
 * M3E migration compatibility surface.
 *
 * This is deliberately broad and temporary. It allows the first
 * genuine large translation-unit split without redesigning behavior.
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

#include "ps2vnc_cross_types.h"

#ifndef VIDEO_MODE_COUNT
#define VIDEO_MODE_COUNT 22u
#endif

/* Main-side macros referenced by services. */
#define OUTPUT_WIDTH   (active_video_mode->raster_width)
#define OUTPUT_HEIGHT  (active_video_mode->raster_height)
#define VNC_WIDTH      RFB_CAPACITY_WIDTH
#define VNC_HEIGHT     RFB_CAPACITY_HEIGHT
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
#define PS2VNC_HTTP_MAX_HEADER      2048
#define PS2VNC_DISPLAY_TX_MAX_BYTES 2048
#define OSK_WIDTH    600
#define OSK_HEIGHT   178
#define RECONNECT_TIMEOUT_SECONDS 10
#define RECONNECT_MAX_ATTEMPTS    10

/* M3E BEGIN GENERATED ENUM COMPATIBILITY */
/* Exact main-TU enum definitions required by detached services. */

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

enum {
    SYSTEM_REFRESH_READY = 0,
    SYSTEM_REFRESH_REFRESHING,
    SYSTEM_REFRESH_COOLDOWN
};
/* M3E END GENERATED ENUM COMPATIBILITY */

#include "ps2vnc_services_imports.h"

#endif

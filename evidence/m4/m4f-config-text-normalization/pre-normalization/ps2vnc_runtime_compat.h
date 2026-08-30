#ifndef PS2VNC_RUNTIME_COMPAT_H
#define PS2VNC_RUNTIME_COMPAT_H

/*
 * M3G shared declaration/type shell.
 * Extracted mechanically from ps2ip.c before runtime state.
 */

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

#endif

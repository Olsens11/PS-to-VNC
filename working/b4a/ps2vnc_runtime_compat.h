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

#include "config/text.h"

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

#include "ps2vnc_video_mode_compat.h"

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

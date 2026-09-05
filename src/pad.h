/*
 * File synopsis:
 * Owns the narrow PS-to-VNC value/lifecycle boundary around direct PS2SDK
 * libpad use for one physical controller endpoint.
 *
 * This file exposes native libpad controller data plus only the immediate
 * physical history PS-to-VNC needs to describe press/release transitions.
 * It does not own pointer behavior, chords, bindings, UI, RFB effects, or
 * haptic policy.
 *
 * Context: docs/adr/0002-use-libpad-directly-reference-opl.md and GitHub
 * Issue #38.
 */

#ifndef PSTVNC_PAD_H
#define PSTVNC_PAD_H

#include <stdint.h>

#include <libpad.h>

typedef struct pstvnc_pad {
    int port;
    int slot;
    int state;

    int opened;
    int connection_configured;

    /*
     * padSetMainMode() starts an asynchronous PADMAN request. A successful
     * request is remembered here so later pstvnc_pad_poll() calls can observe
     * PAD_STATE_EXECCMD without blocking, then verify DualShock mode when the
     * endpoint becomes readable again.
     *
     * Keeping this transition across ordinary polls is important once the pad
     * is owned by a controller thread: no single physical-acquisition call may
     * hide indefinitely inside a mode-settle loop and prevent an ownership or
     * shutdown boundary from being reached.
     */
    int dualshock_mode_request_pending;

    int history_valid;

    /*
     * padRead() may copy fewer bytes than sizeof(struct padButtonStatus).
     * sample_length records how many leading bytes in buttons came from
     * PADMAN. Fields beyond that length must not be treated as observed
     * controller data.
     */
    unsigned char sample_length;
    struct padButtonStatus buttons;

    /*
     * libpad reports button bits active-low in buttons.btns. These three masks
     * are the small PS-to-VNC-owned physical-history convenience: active-high
     * buttons currently down, newly down, and newly up.
     *
     * They deliberately retain the native PAD_* bit vocabulary.
     */
    uint16_t buttons_down;
    uint16_t buttons_pressed;
    uint16_t buttons_released;

    /*
     * libpad/PADMAN DMA storage is caller-owned. Current libpad requires a
     * 256-byte region aligned to 64 bytes for every opened (port, slot).
     */
    unsigned char dma_buffer[256] __attribute__((aligned(64)));
} pstvnc_pad_t;

int pstvnc_pad_init(void);
void pstvnc_pad_shutdown(void);

int pstvnc_pad_open(
    pstvnc_pad_t *pad,
    int port,
    int slot);

int pstvnc_pad_poll(pstvnc_pad_t *pad);

/*
 * End the current physical-controller connection epoch without closing the
 * libpad endpoint.
 *
 * This revokes both controller-mode configuration authority and the remembered
 * sample/edge history. The next usable connection must therefore negotiate or
 * verify its mode and establish a fresh physical baseline before publishing
 * ordinary controller facts.
 *
 * This is the pad owner's hard stale-state boundary used by disconnect,
 * explicit libpad ownership handoff, and similar lifecycle transitions.
 */
void pstvnc_pad_invalidate_connection_epoch(
    pstvnc_pad_t *pad);

void pstvnc_pad_close(pstvnc_pad_t *pad);

#endif

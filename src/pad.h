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
void pstvnc_pad_close(pstvnc_pad_t *pad);

#endif

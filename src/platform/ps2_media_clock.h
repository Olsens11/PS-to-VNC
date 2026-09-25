/*
 * File synopsis:
 * Declares the session-scoped PS2 synchronization/time binding used by the
 * platform-neutral A002 media clock. One binding owns exactly one EE semaphore
 * lifetime and exposes only the existing media-clock synchronization/time
 * observer shapes plus exact current-tick and tick-rate observations.
 *
 * Binding storage is single-session authority: initialize once, release once,
 * then discard that binding object. A replacement Wire/media session constructs
 * a fresh binding object, so stale Session-A observer copies remain terminal.
 *
 * This interface owns no epoch, armed state, deadline arithmetic, media profile,
 * MPEG/AUDIO lifecycle, Application policy, or Wire/protocol behavior.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md;
 * docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A002-PS2-MEDIA-CLOCK-PRODUCT-BINDING-R26.
 */

#ifndef PSTVNC_PS2_MEDIA_CLOCK_H
#define PSTVNC_PS2_MEDIA_CLOCK_H

#include <stdint.h>

#include "media/clock.h"

typedef enum pstvnc_ps2_media_clock_binding_state {
    PSTVNC_PS2_MEDIA_CLOCK_BINDING_NEW = 0,
    PSTVNC_PS2_MEDIA_CLOCK_BINDING_ACTIVE = 1,
    PSTVNC_PS2_MEDIA_CLOCK_BINDING_RETIRED = 2
} pstvnc_ps2_media_clock_binding_state_t;

typedef struct pstvnc_ps2_media_clock_binding {
    int semaphore_id;
    pstvnc_ps2_media_clock_binding_state_t state;
} pstvnc_ps2_media_clock_binding_t;

#define PSTVNC_PS2_MEDIA_CLOCK_BINDING_INITIALIZER \
    { -1, PSTVNC_PS2_MEDIA_CLOCK_BINDING_NEW }

int pstvnc_ps2_media_clock_binding_init(
    pstvnc_ps2_media_clock_binding_t *binding);

int pstvnc_ps2_media_clock_binding_release(
    pstvnc_ps2_media_clock_binding_t *binding);

int pstvnc_ps2_media_clock_binding_sync(
    pstvnc_ps2_media_clock_binding_t *binding,
    pstvnc_media_clock_sync_t *sync);

int pstvnc_ps2_media_clock_binding_time_ops(
    pstvnc_ps2_media_clock_binding_t *binding,
    pstvnc_media_clock_time_ops_t *time_ops);

int pstvnc_ps2_media_clock_binding_tick_rate(
    const pstvnc_ps2_media_clock_binding_t *binding,
    uint32_t *ticks_per_second);

int pstvnc_ps2_media_clock_binding_current_tick(
    const pstvnc_ps2_media_clock_binding_t *binding,
    uint64_t *tick);

#endif /* PSTVNC_PS2_MEDIA_CLOCK_H */

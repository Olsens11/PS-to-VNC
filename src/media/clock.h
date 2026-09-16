/*
 * File synopsis:
 * Defines the clean session-scoped common media clock. One immutable A002
 * media-clock profile supplies epoch lead and signed audio/video offsets; one
 * synchronization contract protects one-shot epoch publication; and narrow
 * timer/delay observers make deadline waiting deterministic and host-testable.
 * This file does not own PCM/AUDSRV runtime or any MPEG/video callsite.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md.
 */

#ifndef PSTVNC_MEDIA_CLOCK_H
#define PSTVNC_MEDIA_CLOCK_H

#include <stdint.h>

#include "config/profile.h"

typedef enum pstvnc_media_clock_result {
    PSTVNC_MEDIA_CLOCK_OK = 0,
    PSTVNC_MEDIA_CLOCK_UNARMED = 1,
    PSTVNC_MEDIA_CLOCK_STOPPED = 2,
    PSTVNC_MEDIA_CLOCK_INVALID = -1,
    PSTVNC_MEDIA_CLOCK_SYNC_FAILED = -2,
    PSTVNC_MEDIA_CLOCK_TIMER_FAILED = -3,
    PSTVNC_MEDIA_CLOCK_DELAY_FAILED = -4
} pstvnc_media_clock_result_t;

/* lock() must acquire and unlock() must release one session clock's state. */
typedef struct pstvnc_media_clock_sync {
    int (*lock)(void *context);
    int (*unlock)(void *context);
    void *context;
} pstvnc_media_clock_sync_t;

/* read_ticks() and delay_us() operate in the tick domain supplied at init. */
typedef struct pstvnc_media_clock_time_ops {
    int (*read_ticks)(void *context, uint64_t *ticks);
    int (*delay_us)(void *context, uint32_t delay_us);
    void *context;
} pstvnc_media_clock_time_ops_t;

/* A null observer means the caller has no cancellation source. */
typedef struct pstvnc_media_clock_stop_observer {
    int (*is_stop_requested)(void *context);
    void *context;
} pstvnc_media_clock_stop_observer_t;

/*
 * After successful init, profile/sync/ticks_per_second are immutable session
 * authority. epoch_tick and armed are clock-private synchronized state;
 * production consumers observe them only through the API below.
 */
typedef struct pstvnc_media_clock {
    pstvnc_config_media_clock_profile_t profile;
    pstvnc_media_clock_sync_t sync;
    uint32_t ticks_per_second;
    uint64_t epoch_tick;
    int armed;
} pstvnc_media_clock_t;

int pstvnc_media_clock_init(
    pstvnc_media_clock_t *clock,
    const pstvnc_config_media_clock_profile_t *profile,
    uint32_t ticks_per_second,
    const pstvnc_media_clock_sync_t *sync);

int pstvnc_media_clock_is_armed(
    const pstvnc_media_clock_t *clock,
    int *armed);

int pstvnc_media_clock_epoch(
    const pstvnc_media_clock_t *clock,
    uint64_t *epoch_tick);

int pstvnc_media_clock_arm(
    pstvnc_media_clock_t *clock,
    uint64_t observed_now_tick);

int pstvnc_media_clock_audio_deadline(
    const pstvnc_media_clock_t *clock,
    uint64_t additional_ticks,
    uint64_t *deadline_tick);

int pstvnc_media_clock_video_deadline(
    const pstvnc_media_clock_t *clock,
    uint64_t additional_ticks,
    uint64_t *deadline_tick);

int pstvnc_media_clock_wait_audio(
    const pstvnc_media_clock_t *clock,
    uint64_t additional_ticks,
    uint32_t poll_us,
    const pstvnc_media_clock_time_ops_t *time_ops,
    const pstvnc_media_clock_stop_observer_t *stop_observer);

int pstvnc_media_clock_wait_video(
    const pstvnc_media_clock_t *clock,
    uint64_t additional_ticks,
    uint32_t poll_us,
    const pstvnc_media_clock_time_ops_t *time_ops,
    const pstvnc_media_clock_stop_observer_t *stop_observer);

#endif

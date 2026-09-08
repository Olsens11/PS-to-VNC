/*
 * File synopsis:
 * Defines H1's P11-derived MPEG-2 video session consumer.
 *
 * The display/IPU/SMS mechanisms remain descendants of the exact tracked P11
 * seed. H1 supplies a mux MPEG queue, session-scoped configuration, shared A/V
 * epoch, and explicit teardown so the ELF can accept another connection.
 */

#ifndef PSTVNC_MEDIA_HARNESS_H1_VIDEO_RUNTIME_H
#define PSTVNC_MEDIA_HARNESS_H1_VIDEO_RUNTIME_H

#include "h1_media_clock.h"
#include "h1_transport_runtime.h"

#include <delaythread.h>
#include <stdint.h>

typedef enum pstvnc_h1_video_error {
    PSTVNC_H1_VIDEO_ERROR_NONE = 0,
    PSTVNC_H1_VIDEO_ERROR_ARGUMENT = 1,
    PSTVNC_H1_VIDEO_ERROR_ALLOCATION = 2,
    PSTVNC_H1_VIDEO_ERROR_PREFILL = 3,
    PSTVNC_H1_VIDEO_ERROR_DECODE = 4,
    PSTVNC_H1_VIDEO_ERROR_DISPLAY = 5,
    PSTVNC_H1_VIDEO_ERROR_DMA = 6
} pstvnc_h1_video_error_t;

typedef struct pstvnc_h1_video_result {
    uint32_t pictures_decoded;
    uint32_t pictures_displayed;
    uint32_t pictures_dropped;

    uint32_t feed_calls;
    uint32_t payload_bytes_submitted;
    uint32_t dma_bytes_submitted;

    uint32_t sequence_callbacks;
    uint32_t sequence_width;
    uint32_t sequence_height;

    uint32_t prefill_target;
    uint32_t prefill_observed;

    uint32_t vsync_waits;
    uint32_t deadline_misses;
    uint64_t max_deadline_late_ticks;

    uint32_t decode_samples;
    uint64_t decode_ticks_total;
    uint64_t decode_ticks_max;

    uint32_t upload_samples;
    uint64_t upload_ticks_total;
    uint64_t upload_ticks_max;

    uint32_t draw_samples;
    uint64_t draw_ticks_total;
    uint64_t draw_ticks_max;

    pstvnc_h1_video_error_t error;
} pstvnc_h1_video_result_t;

int pstvnc_h1_video_chassis_init(void);

int pstvnc_h1_video_run_session(
    pstvnc_h1_transport_runtime_t *transport,
    pstvnc_h1_media_clock_t *clock,
    pstvnc_h1_video_result_t *result);

#endif

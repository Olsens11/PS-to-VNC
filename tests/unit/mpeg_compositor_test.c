/*
 * File synopsis:
 * Host contract tests for A004 P4/P5 neutral macroblock presentation plus
 * exact-run first-sync, retirement drain/seal, and final visible handoff.
 */

#include "display/mpeg_compositor.h"
#include "platform/ps2_graphics.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum platform_mode {
    PLATFORM_OK = 0,
    PLATFORM_FAIL,
    PLATFORM_UNSYNCED,
    PLATFORM_WRONG_TICK_DOMAIN
} platform_mode_t;

static platform_mode_t platform_mode;
static unsigned int platform_calls;
static unsigned int reveal_calls;
static pstvnc_ps2_graphics_video_t last_video;
static uint64_t platform_tick;
static uint32_t platform_ticks_per_second;
static pstvnc_mpeg_presentation_t *sabotage_presentation;

static int sync_lock_fail;
static int sync_unlock_fail;

static int test_lock(void *context)
{
    (void)context;
    return sync_lock_fail ? -1 : 0;
}

static int test_unlock(void *context)
{
    (void)context;
    return sync_unlock_fail ? -1 : 0;
}

int pstvnc_ps2_graphics_present_video_macroblocks(
    const pstvnc_ps2_graphics_video_t *video,
    pstvnc_ps2_graphics_sync_result_t *sync_result)
{
    ++platform_calls;

    if (platform_mode == PLATFORM_FAIL)
        return -1;

    assert(video != NULL);
    assert(sync_result != NULL);

    last_video = *video;
    memset(sync_result, 0, sizeof(*sync_result));

    if (platform_mode != PLATFORM_UNSYNCED) {
        sync_result->synchronized = 1;
        sync_result->observed_sync_tick = platform_tick;
        sync_result->ticks_per_second =
            platform_mode == PLATFORM_WRONG_TICK_DOMAIN
            ? platform_ticks_per_second + 1u
            : platform_ticks_per_second;
    }

    if (sabotage_presentation != NULL)
        sabotage_presentation->run_generation++;

    return 0;
}

int pstvnc_ps2_graphics_reveal_retained_video(
    pstvnc_ps2_graphics_sync_result_t *sync_result)
{
    ++reveal_calls;

    if (platform_mode == PLATFORM_FAIL)
        return -1;

    assert(sync_result != NULL);
    memset(sync_result, 0, sizeof(*sync_result));

    if (platform_mode != PLATFORM_UNSYNCED) {
        sync_result->synchronized = 1;
        sync_result->observed_sync_tick = platform_tick;
        sync_result->ticks_per_second = platform_ticks_per_second;
    }

    return 0;
}

static pstvnc_mpeg_presentation_geometry_t geometry_32x16(void)
{
    pstvnc_mpeg_presentation_geometry_t geometry;

    memset(&geometry, 0, sizeof(geometry));

    geometry.base.x = 100;
    geometry.base.y = 120;
    geometry.base.width = 32;
    geometry.base.height = 16;

    geometry.inner_content.x = 104;
    geometry.inner_content.y = 122;
    geometry.inner_content.width = 24;
    geometry.inner_content.height = 12;

    geometry.suppression.x = 96;
    geometry.suppression.y = 116;
    geometry.suppression.width = 40;
    geometry.suppression.height = 24;

    return geometry;
}

static void init_clock(
    pstvnc_media_clock_t *clock,
    uint32_t lead_us)
{
    pstvnc_config_media_clock_profile_t profile;
    pstvnc_media_clock_sync_t sync;

    memset(&profile, 0, sizeof(profile));
    profile.epoch_lead_us = lead_us;

    memset(&sync, 0, sizeof(sync));
    sync.lock = test_lock;
    sync.unlock = test_unlock;

    assert(pstvnc_media_clock_init(
        clock,
        &profile,
        platform_ticks_per_second,
        &sync) == PSTVNC_MEDIA_CLOCK_OK);
}

static pstvnc_mpeg_compositor_frame_t make_frame(
    const uint16_t *pixels,
    size_t capacity,
    uint32_t generation)
{
    pstvnc_mpeg_compositor_frame_t frame;

    memset(&frame, 0, sizeof(frame));
    frame.surface.pixels = pixels;
    frame.surface.pixel_capacity = capacity;
    frame.surface.width = 32u;
    frame.surface.height = 16u;
    frame.run_generation = generation;
    return frame;
}

static void reset_platform(void)
{
    platform_mode = PLATFORM_OK;
    platform_calls = 0u;
    reveal_calls = 0u;
    memset(&last_video, 0, sizeof(last_video));
    platform_tick = 1000u;
    platform_ticks_per_second = 1000000u;
    sabotage_presentation = NULL;
    sync_lock_fail = 0;
    sync_unlock_fail = 0;
}

static void test_macroblock_detile(void)
{
    uint16_t source[512];
    uint16_t linear[512];
    pstvnc_mpeg_rgb16_macroblock_surface_t surface;
    unsigned int row;
    unsigned int column;

    for (row = 0u; row < 16u; ++row) {
        for (column = 0u; column < 16u; ++column) {
            source[(size_t)row * 16u + column] =
                (uint16_t)(1000u + row * 16u + column);

            source[256u + (size_t)row * 16u + column] =
                (uint16_t)(2000u + row * 16u + column);
        }
    }

    memset(&surface, 0, sizeof(surface));
    surface.pixels = source;
    surface.pixel_capacity = 512u;
    surface.width = 32u;
    surface.height = 16u;

    assert(pstvnc_mpeg_rgb16_macroblock_surface_valid(&surface));
    assert(pstvnc_mpeg_rgb16_detile(
        &surface,
        linear,
        512u));

    for (row = 0u; row < 16u; ++row) {
        for (column = 0u; column < 16u; ++column) {
            assert(linear[(size_t)row * 32u + column] ==
                source[(size_t)row * 16u + column]);

            assert(linear[(size_t)row * 32u + 16u + column] ==
                source[256u + (size_t)row * 16u + column]);
        }
    }

    assert(!pstvnc_mpeg_rgb16_detile(
        &surface,
        linear,
        511u));

    surface.width = 31u;
    assert(!pstvnc_mpeg_rgb16_macroblock_surface_valid(&surface));
}

static void test_first_sync_arms_and_promotes(void)
{
    uint16_t pixels[512] = { 0u };
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t geometry = geometry_32x16();
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_compositor_frame_t frame;
    pstvnc_mpeg_compositor_effects_t effects;
    uint64_t epoch;

    reset_platform();
    init_clock(&clock, 100u);

    pstvnc_mpeg_presentation_init(&presentation);
    assert(pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        7u));

    frame = make_frame(pixels, 512u, 7u);

    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_OK);

    assert(platform_calls == 1u);
    assert(last_video.surface.pixels == pixels);
    assert(last_video.surface.width == 32u);
    assert(last_video.surface.height == 16u);
    assert(last_video.base.x == geometry.base.x);
    assert(last_video.base.y == geometry.base.y);
    assert(last_video.inner_content.x == geometry.inner_content.x);
    assert(last_video.suppression.x == geometry.suppression.x);

    assert(effects.synchronized);
    assert(effects.clock_armed_now);
    assert(effects.first_frame_promoted);
    assert(effects.observed_sync_tick == 1000u);

    assert(pstvnc_media_clock_epoch(
        &clock,
        &epoch) == PSTVNC_MEDIA_CLOCK_OK);
    assert(epoch == 1100u);

    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_MPEG_OWNED);
}

static void test_subsequent_frame_preserves_epoch(void)
{
    uint16_t pixels[512] = { 0u };
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t geometry = geometry_32x16();
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_compositor_frame_t frame;
    pstvnc_mpeg_compositor_effects_t effects;
    uint64_t first_epoch;
    uint64_t second_epoch;

    reset_platform();
    init_clock(&clock, 50u);

    pstvnc_mpeg_presentation_init(&presentation);
    assert(pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        11u));

    frame = make_frame(pixels, 512u, 11u);

    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_OK);

    assert(pstvnc_media_clock_epoch(
        &clock,
        &first_epoch) == PSTVNC_MEDIA_CLOCK_OK);

    platform_tick = 9000u;

    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_OK);

    assert(effects.synchronized);
    assert(!effects.clock_armed_now);
    assert(!effects.first_frame_promoted);

    assert(pstvnc_media_clock_epoch(
        &clock,
        &second_epoch) == PSTVNC_MEDIA_CLOCK_OK);
    assert(second_epoch == first_epoch);
    assert(platform_calls == 2u);
}

static void test_later_run_preserves_session_epoch(void)
{
    uint16_t pixels[512] = { 0u };
    pstvnc_mpeg_presentation_t first;
    pstvnc_mpeg_presentation_t later;
    pstvnc_mpeg_presentation_geometry_t geometry = geometry_32x16();
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_compositor_frame_t frame;
    pstvnc_mpeg_compositor_effects_t effects;
    uint64_t epoch_before;
    uint64_t epoch_after;

    reset_platform();
    init_clock(&clock, 25u);

    pstvnc_mpeg_presentation_init(&first);
    assert(pstvnc_mpeg_presentation_arm(
        &first,
        &geometry,
        20u));

    frame = make_frame(pixels, 512u, 20u);
    assert(pstvnc_mpeg_compositor_present(
        &first,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_OK);

    assert(pstvnc_media_clock_epoch(
        &clock,
        &epoch_before) == PSTVNC_MEDIA_CLOCK_OK);

    assert(pstvnc_mpeg_presentation_begin_retirement(
        &first,
        20u));
    assert(pstvnc_mpeg_presentation_seal_retirement(
        &first,
        20u));
    assert(pstvnc_mpeg_compositor_reveal_retired(
        &first,
        20u,
        &effects) == PSTVNC_MPEG_COMPOSITOR_OK);
    assert(pstvnc_mpeg_presentation_state(&first) ==
        PSTVNC_MPEG_PRESENTATION_RFB_ONLY);

    /*
     * A later distinct run reuses the same session clock after a complete P5
     * visible retirement. The prior session epoch must remain unchanged.
     */
    pstvnc_mpeg_presentation_init(&later);
    assert(pstvnc_mpeg_presentation_arm(
        &later,
        &geometry,
        21u));

    platform_tick = 5000u;
    frame.run_generation = 21u;

    assert(pstvnc_mpeg_compositor_present(
        &later,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_OK);

    assert(!effects.clock_armed_now);
    assert(effects.first_frame_promoted);

    assert(pstvnc_media_clock_epoch(
        &clock,
        &epoch_after) == PSTVNC_MEDIA_CLOCK_OK);
    assert(epoch_after == epoch_before);
}

static void test_stale_and_geometry_mismatch_fail_before_platform(void)
{
    uint16_t pixels[512] = { 0u };
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t geometry = geometry_32x16();
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_compositor_frame_t frame;
    pstvnc_mpeg_compositor_effects_t effects;

    reset_platform();
    init_clock(&clock, 0u);

    pstvnc_mpeg_presentation_init(&presentation);
    assert(pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        30u));

    frame = make_frame(pixels, 512u, 29u);

    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_INVALID);
    assert(platform_calls == 0u);

    frame.run_generation = 30u;
    frame.surface.width = 16u;
    frame.surface.pixel_capacity = 512u;

    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_INVALID);
    assert(platform_calls == 0u);
}

static void test_platform_and_sync_fail_closed(void)
{
    uint16_t pixels[512] = { 0u };
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t geometry = geometry_32x16();
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_compositor_frame_t frame;
    pstvnc_mpeg_compositor_effects_t effects;
    int armed;

    reset_platform();
    init_clock(&clock, 0u);

    pstvnc_mpeg_presentation_init(&presentation);
    assert(pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        40u));

    frame = make_frame(pixels, 512u, 40u);

    platform_mode = PLATFORM_FAIL;

    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_PLATFORM_FAILED);
    assert(!effects.synchronized);

    platform_mode = PLATFORM_UNSYNCED;

    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_SYNC_INVALID);
    assert(!effects.synchronized);

    platform_mode = PLATFORM_WRONG_TICK_DOMAIN;

    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_SYNC_INVALID);

    assert(pstvnc_media_clock_is_armed(
        &clock,
        &armed) == PSTVNC_MEDIA_CLOCK_OK);
    assert(!armed);
    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME);
}

static void test_clock_failure_after_sync_does_not_promote(void)
{
    uint16_t pixels[512] = { 0u };
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t geometry = geometry_32x16();
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_compositor_frame_t frame;
    pstvnc_mpeg_compositor_effects_t effects;

    reset_platform();
    init_clock(&clock, 0u);

    pstvnc_mpeg_presentation_init(&presentation);
    assert(pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        50u));

    frame = make_frame(pixels, 512u, 50u);
    sync_lock_fail = 1;

    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_CLOCK_FAILED);

    assert(effects.synchronized);
    assert(!effects.first_frame_promoted);
    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME);
}


static void test_full_inner_matte_remains_presentable(void)
{
    uint16_t pixels[512] = { 0u };
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t geometry = geometry_32x16();
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_compositor_frame_t frame;
    pstvnc_mpeg_compositor_effects_t effects;

    reset_platform();
    init_clock(&clock, 0u);

    geometry.inner_content.x =
        geometry.base.x + geometry.base.width / 2;
    geometry.inner_content.y =
        geometry.base.y + geometry.base.height / 2;
    geometry.inner_content.width = 0;
    geometry.inner_content.height = 0;

    pstvnc_mpeg_presentation_init(&presentation);
    assert(pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        70u));

    frame = make_frame(pixels, 512u, 70u);

    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_OK);

    assert(platform_calls == 1u);
    assert(last_video.inner_content.width == 0);
    assert(last_video.inner_content.height == 0);
    assert(effects.first_frame_promoted);
}


static void test_retirement_drains_then_seals_and_reveals_exact_run(void)
{
    uint16_t pixels[512] = { 0u };
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t geometry = geometry_32x16();
    pstvnc_mpeg_presentation_geometry_t snapshot;
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_compositor_frame_t frame;
    pstvnc_mpeg_compositor_effects_t effects;
    uint32_t snapshot_generation = 0u;
    uint64_t epoch_before;
    uint64_t epoch_after;
    unsigned int calls_before_seal;

    reset_platform();
    init_clock(&clock, 25u);

    pstvnc_mpeg_presentation_init(&presentation);
    assert(pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        80u));

    frame = make_frame(pixels, 512u, 80u);
    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_OK);
    assert(pstvnc_media_clock_epoch(
        &clock,
        &epoch_before) == PSTVNC_MEDIA_CLOCK_OK);

    assert(pstvnc_mpeg_presentation_begin_retirement(
        &presentation,
        80u));
    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_RETIRING);

    platform_tick = 2000u;
    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_OK);
    assert(effects.synchronized);
    assert(!effects.clock_armed_now);
    assert(!effects.first_frame_promoted);
    assert(!effects.retirement_revealed);
    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_RETIRING);

    frame.run_generation = 81u;
    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_INVALID);
    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_RETIRING);

    frame.run_generation = 80u;
    calls_before_seal = platform_calls;

    assert(pstvnc_mpeg_presentation_seal_retirement(
        &presentation,
        80u));
    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING);

    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_INVALID);
    assert(platform_calls == calls_before_seal);

    assert(pstvnc_mpeg_presentation_snapshot(
        &presentation,
        &snapshot,
        &snapshot_generation));
    assert(snapshot_generation == 80u);

    assert(pstvnc_mpeg_compositor_reveal_retired(
        &presentation,
        79u,
        &effects) == PSTVNC_MPEG_COMPOSITOR_INVALID);
    assert(reveal_calls == 0u);

    platform_mode = PLATFORM_FAIL;
    assert(pstvnc_mpeg_compositor_reveal_retired(
        &presentation,
        80u,
        &effects) == PSTVNC_MPEG_COMPOSITOR_PLATFORM_FAILED);
    assert(reveal_calls == 1u);
    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING);
    assert(pstvnc_mpeg_presentation_snapshot(
        &presentation,
        &snapshot,
        &snapshot_generation));

    platform_mode = PLATFORM_UNSYNCED;
    assert(pstvnc_mpeg_compositor_reveal_retired(
        &presentation,
        80u,
        &effects) == PSTVNC_MPEG_COMPOSITOR_SYNC_INVALID);
    assert(reveal_calls == 2u);
    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_REVEAL_PENDING);

    platform_mode = PLATFORM_OK;
    platform_tick = 3000u;
    assert(pstvnc_mpeg_compositor_reveal_retired(
        &presentation,
        80u,
        &effects) == PSTVNC_MPEG_COMPOSITOR_OK);
    assert(reveal_calls == 3u);
    assert(effects.synchronized);
    assert(effects.retirement_revealed);
    assert(effects.observed_sync_tick == 3000u);
    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_RFB_ONLY);
    assert(!pstvnc_mpeg_presentation_snapshot(
        &presentation,
        &snapshot,
        &snapshot_generation));

    assert(pstvnc_media_clock_epoch(
        &clock,
        &epoch_after) == PSTVNC_MEDIA_CLOCK_OK);
    assert(epoch_after == epoch_before);

    assert(pstvnc_mpeg_compositor_reveal_retired(
        &presentation,
        80u,
        &effects) == PSTVNC_MPEG_COMPOSITOR_INVALID);
    assert(reveal_calls == 3u);
}

static void test_promotion_failure_is_not_reported_success(void)
{
    uint16_t pixels[512] = { 0u };
    pstvnc_mpeg_presentation_t presentation;
    pstvnc_mpeg_presentation_geometry_t geometry = geometry_32x16();
    pstvnc_media_clock_t clock;
    pstvnc_mpeg_compositor_frame_t frame;
    pstvnc_mpeg_compositor_effects_t effects;

    reset_platform();
    init_clock(&clock, 0u);

    pstvnc_mpeg_presentation_init(&presentation);
    assert(pstvnc_mpeg_presentation_arm(
        &presentation,
        &geometry,
        60u));

    frame = make_frame(pixels, 512u, 60u);
    sabotage_presentation = &presentation;

    assert(pstvnc_mpeg_compositor_present(
        &presentation,
        &clock,
        &frame,
        &effects) == PSTVNC_MPEG_COMPOSITOR_PROMOTION_FAILED);

    assert(effects.synchronized);
    assert(effects.clock_armed_now);
    assert(!effects.first_frame_promoted);
    assert(pstvnc_mpeg_presentation_state(&presentation) ==
        PSTVNC_MPEG_PRESENTATION_WAIT_FIRST_FRAME);
}

int main(void)
{
    test_macroblock_detile();
    test_first_sync_arms_and_promotes();
    test_subsequent_frame_preserves_epoch();
    test_later_run_preserves_session_epoch();
    test_stale_and_geometry_mismatch_fail_before_platform();
    test_platform_and_sync_fail_closed();
    test_clock_failure_after_sync_does_not_promote();
    test_full_inner_matte_remains_presentable();
    test_retirement_drains_then_seals_and_reveals_exact_run();
    test_promotion_failure_is_not_reported_success();

    puts("MPEG_COMPOSITOR_TEST=PASS");
    return 0;
}

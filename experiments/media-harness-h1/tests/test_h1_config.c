/*
 * File synopsis:
 * Host-side structural tests for H1 CONFIG v3.
 *
 * These tests intentionally include aggressive queue sizes to guard the H1
 * rule that the configuration layer must not substitute a guessed safe
 * capacity ceiling for actual PS2 allocation/hardware evidence.
 *
 * CONFIG v3 also proves the explicit RFB-off seam and 16-pixel-grid MPEG
 * encode/presentation geometry used by cumulative integration.
 */

#include "h1_config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint32_t raw_i32(int32_t value)
{
    uint32_t raw;
    memcpy(&raw, &value, sizeof(raw));
    return raw;
}

static pstvnc_h1_config_t p11_video_profile(void)
{
    pstvnc_h1_config_t c;

    memset(&c, 0, sizeof(c));
    c.version = PSTVNC_H1_CONFIG_VERSION;
    c.profile_id = 0u;
    c.session_id = 1u;

    c.audio_mode = PSTVNC_H1_AUDIO_OFF;
    c.video_mode = PSTVNC_H1_VIDEO_MPEG2_ES;
    c.rfb_mode = PSTVNC_H1_RFB_OFF;

    c.audio_queue_capacity = 0u;
    c.mpeg_queue_capacity = 524288u;

    c.audio_credit_batch_bytes = 0u;
    c.mpeg_credit_batch_bytes = 8192u;
    c.audio_credit_flush_on_empty = 0u;
    c.mpeg_credit_flush_on_empty = 1u;
    c.audio_credit_return_enabled = 0u;
    c.mpeg_credit_return_enabled = 1u;
    c.audio_initial_credit_bytes = 0u;
    c.mpeg_initial_credit_bytes = 524288u;

    c.audio_start_mode = PSTVNC_H1_AUDIO_START_IMMEDIATE;
    c.audio_start_target_bytes = 0u;
    c.audio_start_delay_us = 0u;
    c.audio_chunk_bytes = 0u;
    c.audio_idle_delay_us = 0u;
    c.audio_thread_priority = 0u;
    c.audio_thread_stack_size = 0u;
    c.audio_rate = 0u;
    c.audio_channels = 0u;
    c.audio_bits = 0u;
    c.audio_volume = 0u;
    c.audio_presentation_offset_us = raw_i32(0);

    c.mpeg_start_target_bytes = 458752u;
    c.mpeg_prefill_wait_us = 1000u;
    c.mpeg_prefill_max_loops = 10000u;
    c.mpeg_empty_delay_us = 1000u;
    c.mpeg_feed_bytes = 2048u;

    c.video_fps_num = 30000u;
    c.video_fps_den = 1001u;
    c.video_scheduler_mode = PSTVNC_H1_VIDEO_SCHED_ABSOLUTE;
    c.video_presentation_offset_us = raw_i32(0);
    c.video_pixel_mode = PSTVNC_H1_VIDEO_RGB16;
    c.video_max_width = 704u;
    c.video_max_height = 480u;
    c.video_encode_width = 608u;
    c.video_encode_height = 416u;
    c.video_draw_width = 640u;
    c.video_draw_height = 512u;
    c.video_draw_x = 0u;
    c.video_draw_y = 0u;
    c.video_stage_markers = 0u;
    c.video_stage_hold_vsyncs = 0u;
    c.video_ipu_reset_each_session = 1u;
    c.video_drop_enabled = 0u;
    c.video_drop_threshold_milliframes = 0u;

    c.receiver_thread_priority = 63u;
    c.receiver_thread_stack_size = 16384u;
    c.max_data_payload = 8192u;
    c.socket_receive_buffer_bytes = 0u;
    c.socket_send_buffer_bytes = 0u;
    c.queue_allocation_order = PSTVNC_H1_ALLOCATE_MPEG_FIRST;
    c.media_epoch_lead_us = 0u;

    return c;
}

static pstvnc_h1_config_t combined_pcm_profile(void)
{
    pstvnc_h1_config_t c = p11_video_profile();

    c.profile_id = 1u;
    c.audio_mode = PSTVNC_H1_AUDIO_PCM;
    c.audio_queue_capacity = 524288u;
    c.audio_credit_batch_bytes = 4096u;
    c.audio_credit_flush_on_empty = 1u;
    c.audio_credit_return_enabled = 1u;
    c.audio_initial_credit_bytes = 524288u;
    c.audio_start_mode = PSTVNC_H1_AUDIO_START_TARGET;
    c.audio_start_target_bytes = 458752u;
    c.audio_chunk_bytes = 4096u;
    c.audio_idle_delay_us = 1000u;
    c.audio_thread_priority = 65u;
    c.audio_thread_stack_size = 16384u;
    c.audio_rate = 48000u;
    c.audio_channels = 2u;
    c.audio_bits = 16u;
    c.audio_volume = 100u;
    c.audio_presentation_offset_us = raw_i32(0);
    c.queue_allocation_order = PSTVNC_H1_ALLOCATE_AUDIO_FIRST;

    return c;
}

int main(void)
{
    pstvnc_h1_config_t c;

    c = p11_video_profile();
    assert(pstvnc_h1_config_validate(&c));
    assert(pstvnc_h1_config_audio_offset_us(&c) == 0);
    assert(pstvnc_h1_config_video_offset_us(&c) == 0);

    c = combined_pcm_profile();
    assert(pstvnc_h1_config_validate(&c));
    assert(pstvnc_h1_config_audio_frame_bytes(&c) == 4u);

    c.audio_presentation_offset_us = raw_i32(-43000);
    c.video_presentation_offset_us = raw_i32(12000);
    assert(pstvnc_h1_config_validate(&c));
    assert(pstvnc_h1_config_audio_offset_us(&c) == -43000);
    assert(pstvnc_h1_config_video_offset_us(&c) == 12000);

    /* No experiment-defined queue ceiling: structural consistency still wins. */
    c.audio_queue_capacity = 12u * 1024u * 1024u;
    c.audio_initial_credit_bytes = c.audio_queue_capacity;
    c.audio_start_target_bytes = 10u * 1024u * 1024u;
    c.mpeg_queue_capacity = 16u * 1024u * 1024u;
    c.mpeg_initial_credit_bytes = c.mpeg_queue_capacity;
    c.mpeg_start_target_bytes = 14u * 1024u * 1024u;
    assert(pstvnc_h1_config_validate(&c));
    puts("H1_NO_ARTIFICIAL_QUEUE_CEILING=PASS");

    c = combined_pcm_profile();
    c.max_data_payload = 8193u;
    assert(!pstvnc_h1_config_validate(&c));

    c = combined_pcm_profile();
    c.receiver_thread_priority = 128u;
    assert(!pstvnc_h1_config_validate(&c));

    c = combined_pcm_profile();
    c.mpeg_start_target_bytes = c.mpeg_queue_capacity + 1u;
    assert(!pstvnc_h1_config_validate(&c));

    c = combined_pcm_profile();
    c.mpeg_feed_bytes = 0u;
    assert(!pstvnc_h1_config_validate(&c));

    c = combined_pcm_profile();
    c.video_max_width = 703u;
    assert(!pstvnc_h1_config_validate(&c));

    c = combined_pcm_profile();
    c.video_encode_width = 610u;
    assert(!pstvnc_h1_config_validate(&c));

    c = combined_pcm_profile();
    c.video_draw_x = 1u;
    assert(!pstvnc_h1_config_validate(&c));

    c = combined_pcm_profile();
    c.rfb_mode = PSTVNC_H1_RFB_ON_RESERVED;
    assert(!pstvnc_h1_config_validate(&c));

    c = p11_video_profile();
    c.audio_queue_capacity = 4096u;
    assert(!pstvnc_h1_config_validate(&c));

    puts("H1_CONFIG_HOST_TEST=PASS");
    return 0;
}

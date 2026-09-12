/* Host contract for CP2P item #10's exact public CONFIG gate. */
#include "h1_config.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static pstvnc_h1_config_t make_base(void)
{
    pstvnc_h1_config_t config;

    memset(&config, 0, sizeof(config));
    config.version = PSTVNC_H1_CONFIG_VERSION;
    config.audio_mode = PSTVNC_H1_AUDIO_OFF;
    config.video_mode = PSTVNC_H1_VIDEO_OFF;
    config.receiver_thread_priority = 63u;
    config.receiver_thread_stack_size = 16384u;
    config.max_data_payload = 8192u;
    config.queue_allocation_order = PSTVNC_H1_ALLOCATE_AUDIO_FIRST;
    config.rfb_mode = PSTVNC_H1_RFB_OFF;
    return config;
}

static void enable_rfb_visible(pstvnc_h1_config_t *config)
{
    config->rfb_mode = PSTVNC_H1_RFB_ON_VISIBLE;
    config->rfb_queue_capacity = 32768u;
    config->rfb_credit_batch_bytes = 8192u;
    config->rfb_credit_flush_on_empty = 1u;
    config->rfb_credit_return_enabled = 1u;
    config->rfb_initial_credit_bytes = 32768u;
}

static void enable_pcm(pstvnc_h1_config_t *config)
{
    config->audio_mode = PSTVNC_H1_AUDIO_PCM;
    config->audio_queue_capacity = 524288u;
    config->audio_credit_batch_bytes = 4096u;
    config->audio_credit_flush_on_empty = 1u;
    config->audio_credit_return_enabled = 1u;
    config->audio_initial_credit_bytes = 524288u;
    config->audio_start_mode = PSTVNC_H1_AUDIO_START_TARGET;
    config->audio_start_target_bytes = 458752u;
    config->audio_chunk_bytes = 4096u;
    config->audio_idle_delay_us = 1000u;
    config->audio_thread_priority = 65u;
    config->audio_thread_stack_size = 16384u;
    config->audio_rate = 48000u;
    config->audio_channels = 2u;
    config->audio_bits = 16u;
    config->audio_volume = 100u;
}

static void enable_mpeg(pstvnc_h1_config_t *config)
{
    config->video_mode = PSTVNC_H1_VIDEO_MPEG2_ES;
    config->mpeg_queue_capacity = 524288u;
    config->mpeg_credit_batch_bytes = 8192u;
    config->mpeg_credit_flush_on_empty = 1u;
    config->mpeg_credit_return_enabled = 1u;
    config->mpeg_initial_credit_bytes = 524288u;
    config->mpeg_start_target_bytes = 458752u;
    config->mpeg_prefill_wait_us = 1000u;
    config->mpeg_prefill_max_loops = 10000u;
    config->mpeg_empty_delay_us = 1000u;
    config->mpeg_feed_bytes = 2048u;
    config->video_fps_num = 30000u;
    config->video_fps_den = 1001u;
    config->video_scheduler_mode = PSTVNC_H1_VIDEO_SCHED_ABSOLUTE;
    config->video_pixel_mode = PSTVNC_H1_VIDEO_RGB16;
    config->video_max_width = 704u;
    config->video_max_height = 480u;
    config->video_encode_width = 608u;
    config->video_encode_height = 416u;
    config->video_draw_width = 640u;
    config->video_draw_height = 512u;
    config->video_ipu_reset_each_session = 1u;
}

int main(void)
{
    pstvnc_h1_config_t config;

    config = make_base();
    assert(pstvnc_h1_config_validate(&config));

    config = make_base();
    enable_rfb_visible(&config);
    enable_mpeg(&config);
    assert(pstvnc_h1_config_validate(&config));

    config = make_base();
    enable_rfb_visible(&config);
    enable_mpeg(&config);
    enable_pcm(&config);
    assert(pstvnc_h1_config_validate(&config));

    config = make_base();
    enable_rfb_visible(&config);
    enable_pcm(&config);
    assert(!pstvnc_h1_config_validate(&config));

    config = make_base();
    enable_rfb_visible(&config);
    enable_mpeg(&config);
    config.rfb_mode = PSTVNC_H1_RFB_ON_RESERVED;
    assert(!pstvnc_h1_config_validate(&config));

    config = make_base();
    enable_rfb_visible(&config);
    enable_mpeg(&config);
    enable_pcm(&config);
    config.mpeg_queue_capacity = 4096u;
    assert(!pstvnc_h1_config_validate(&config));

    config = make_base();
    enable_rfb_visible(&config);
    enable_mpeg(&config);
    enable_pcm(&config);
    config.video_max_width = 700u;
    assert(!pstvnc_h1_config_validate(&config));

    config = make_base();
    enable_rfb_visible(&config);
    enable_mpeg(&config);
    enable_pcm(&config);
    config.rfb_initial_credit_bytes = 32769u;
    assert(!pstvnc_h1_config_validate(&config));

    puts("H1_CP2P_ITEM10_CONFIG_GATE_HOST_TEST=PASS");
    return 0;
}

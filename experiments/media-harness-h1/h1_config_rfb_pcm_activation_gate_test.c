/*
 * Host test for the CP2O cumulative CONFIG activation wrapper.
 *
 * The new gate must preserve already-qualified RFB-only acceptance, open exactly
 * visible RFB + canonical PCM with MPEG OFF, and continue delegating all PCM and
 * ordinary H1 field validation to the unchanged inner validator.
 */

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

static pstvnc_h1_config_t make_rfb(uint32_t mode)
{
    pstvnc_h1_config_t config = make_base();

    config.rfb_mode = mode;
    config.rfb_queue_capacity = 32768u;
    config.rfb_credit_batch_bytes = 8192u;
    config.rfb_credit_flush_on_empty = 1u;
    config.rfb_credit_return_enabled = 1u;
    config.rfb_initial_credit_bytes = 32768u;
    return config;
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
    config->audio_start_delay_us = 0u;
    config->audio_chunk_bytes = 4096u;
    config->audio_idle_delay_us = 1000u;
    config->audio_thread_priority = 8u;
    config->audio_thread_stack_size = 16384u;
    config->audio_rate = 48000u;
    config->audio_channels = 2u;
    config->audio_bits = 16u;
    config->audio_volume = 100u;
    config->audio_presentation_offset_us = 0u;
}

int main(void)
{
    pstvnc_h1_config_t config;

    config = make_base();
    assert(pstvnc_h1_config_validate(&config));

    config = make_rfb(PSTVNC_H1_RFB_ON_RESERVED);
    assert(pstvnc_h1_config_validate(&config));

    config = make_rfb(PSTVNC_H1_RFB_ON_VISIBLE);
    assert(pstvnc_h1_config_validate(&config));

    config = make_rfb(PSTVNC_H1_RFB_ON_VISIBLE);
    enable_pcm(&config);
    assert(pstvnc_h1_config_validate(&config));

    config = make_rfb(PSTVNC_H1_RFB_ON_RESERVED);
    enable_pcm(&config);
    assert(!pstvnc_h1_config_validate(&config));

    config = make_rfb(PSTVNC_H1_RFB_ON_VISIBLE);
    enable_pcm(&config);
    config.video_mode = PSTVNC_H1_VIDEO_MPEG2_ES;
    assert(!pstvnc_h1_config_validate(&config));

    config = make_rfb(PSTVNC_H1_RFB_ON_VISIBLE);
    enable_pcm(&config);
    config.audio_queue_capacity = 4096u;
    assert(!pstvnc_h1_config_validate(&config));

    config = make_rfb(PSTVNC_H1_RFB_ON_VISIBLE);
    enable_pcm(&config);
    config.audio_thread_priority = 0u;
    assert(!pstvnc_h1_config_validate(&config));

    config = make_rfb(PSTVNC_H1_RFB_ON_VISIBLE);
    enable_pcm(&config);
    config.rfb_initial_credit_bytes = 32769u;
    assert(!pstvnc_h1_config_validate(&config));

    printf("H1_RFB_PCM_CONFIG_ACTIVATION_GATE_TEST=PASS\n");
    return 0;
}

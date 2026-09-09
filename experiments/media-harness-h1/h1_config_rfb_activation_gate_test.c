/*
 * Host test for the cumulative-only RFB CONFIG activation wrapper.
 *
 * Compile h1_config.c with pstvnc_h1_config_validate renamed to the inner symbol,
 * then link this test with h1_config_rfb_activation_gate.c. This proves the
 * wrapper opens exactly the headless/visible RFB-only modes while retaining the
 * ordinary validator for every non-RFB field.
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

static pstvnc_h1_config_t make_rfb_only(uint32_t rfb_mode)
{
    pstvnc_h1_config_t config = make_base();

    config.rfb_mode = rfb_mode;
    config.rfb_queue_capacity = 32768u;
    config.rfb_credit_batch_bytes = 8192u;
    config.rfb_credit_flush_on_empty = 1u;
    config.rfb_credit_return_enabled = 1u;
    config.rfb_initial_credit_bytes = 32768u;
    return config;
}

int main(void)
{
    pstvnc_h1_config_t config;

    config = make_base();
    assert(pstvnc_h1_config_validate(&config));

    config = make_rfb_only(PSTVNC_H1_RFB_ON_RESERVED);
    assert(pstvnc_h1_config_validate(&config));

    config = make_rfb_only(PSTVNC_H1_RFB_ON_VISIBLE);
    assert(pstvnc_h1_config_validate(&config));

    config = make_rfb_only(PSTVNC_H1_RFB_ON_VISIBLE);
    config.audio_mode = PSTVNC_H1_AUDIO_PCM;
    assert(!pstvnc_h1_config_validate(&config));

    config = make_rfb_only(PSTVNC_H1_RFB_ON_VISIBLE);
    config.video_mode = PSTVNC_H1_VIDEO_MPEG2_ES;
    assert(!pstvnc_h1_config_validate(&config));

    config = make_rfb_only(3u);
    assert(!pstvnc_h1_config_validate(&config));

    config = make_rfb_only(PSTVNC_H1_RFB_ON_VISIBLE);
    config.rfb_queue_capacity = 4096u;
    assert(!pstvnc_h1_config_validate(&config));

    config = make_rfb_only(PSTVNC_H1_RFB_ON_VISIBLE);
    config.rfb_initial_credit_bytes = 32769u;
    assert(!pstvnc_h1_config_validate(&config));

    config = make_rfb_only(PSTVNC_H1_RFB_ON_VISIBLE);
    config.rfb_credit_return_enabled = 0u;
    assert(!pstvnc_h1_config_validate(&config));

    /* Inner validator must still reject an unrelated ordinary H1 error. */
    config = make_rfb_only(PSTVNC_H1_RFB_ON_VISIBLE);
    config.receiver_thread_priority = 0u;
    assert(!pstvnc_h1_config_validate(&config));

    printf("H1_RFB_CONFIG_ACTIVATION_GATE_TEST=PASS\n");
    return 0;
}